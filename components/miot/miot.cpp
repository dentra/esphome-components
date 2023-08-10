#include "esphome/core/log.h"
#include "mbedtls/ccm.h"
#include "inttypes.h"

#include "miot.h"
#include "miot_decrypt.h"

namespace esphome {
namespace miot {

static const char *const TAG = "miot";

static inline const char *get_signal_bars(int rssi) {
  if (rssi >= -50) {
    return "excellent";
  }
  if (rssi >= -65) {
    return "good";
  }
  if (rssi >= -85) {
    return "normal";
  }
  return "poor";
}

bool MiBeaconTracker::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  bool processed = false;
  for (auto &service_data : device.get_service_datas()) {
    if (service_data.uuid.contains(0x95, 0xFE)) {
      return this->parse_mibeacon_(device, service_data.data);
    }
  }
  return false;
}

bool MiBeaconTracker::parse_mibeacon_(const esp32_ble_tracker::ESPBTDevice &device,
                                      const std::vector<uint8_t> &raw) const {
  if (raw.size() < sizeof(RawMiBeaconHeader)) {
    ESP_LOGW(TAG, "Invalid MiBeacon data size: %s", format_hex_pretty(raw).c_str());
    return false;
  }

  auto mib = MiBeacon(reinterpret_cast<const RawMiBeaconHeader *>(raw.data()));

  if (mib.frame_control.mesh) {
    // ESP_LOGW(TAG, "Device data is a mesh type device: %s", format_hex_pretty(raw).c_str());
    return false;
  }

  if (mib.frame_control.version < 2) {
    ESP_LOGW(TAG, "MiBeacon is using old data format %" PRIu8 ": %s", mib.frame_control.version,
             format_hex_pretty(raw).c_str());
    return false;
  }

  auto data = raw.data() + sizeof(RawMiBeaconHeader);

  if (mib.frame_control.mac_include) {
    mib.mac_address =
        (*reinterpret_cast<const uint64_t *>(data)) & 0x0000FFFFFFFFFFFFul;  // mac address 6 bytes + 2 bytes for uint64
    data = data + sizeof(esp_bd_addr_t);

    if (mib.mac_address != device.address_uint64()) {
      MIOT_LOGW(TAG, "Device MAC address doesn't match mibeacon MAC address %12" PRIX64, device.address_uint64(),
                mib.mac_address);
      return false;
    }
  } else {
    mib.mac_address = device.address_uint64();
  }

  if (mib.frame_control.capability_include) {
    mib.capability = *reinterpret_cast<const Capability *>(data);
    data = data + sizeof(Capability);
    if (mib.capability.io) {
      mib.io_capability = *reinterpret_cast<const IOCapability *>(data);
      data = data + sizeof(IOCapability);
    }
  }

  if (mib.frame_control.object_include) {
    if (mib.frame_control.is_encrypted) {
      const uint8_t *end = raw.data() + raw.size();
      const uint8_t *mic;
      if (mib.frame_control.version > 3) {
        mic = end - sizeof(uint32_t);
        mib.message_integrity_check = *reinterpret_cast<const uint32_t *>(mic);
      } else {
        mic = end - sizeof(uint8_t);
        mib.message_integrity_check = *reinterpret_cast<const uint8_t *>(mic);
      }
      const uint8_t *rnd = mic - sizeof(uint32_t);
      // random_number combined with frame_counter to become a 4-byte Counter for anti-replay
      mib.random_number = ((*reinterpret_cast<const uint32_t *>(rnd)) & 0xFFFFFF00) | mib.frame_counter;
      rnd++;  // random number size is 3 bytes length, so add one.
      mib.object = BLEObject(data, rnd);
    } else {
      mib.object = BLEObject((RawBLEObject *) data);
    }
  }

  // parsing done. now processing...

  bool processed = false;
  for (auto listener : this->listeners_) {
    if (device.address_uint64() != listener->get_address()) {
      continue;
    }
    if (mib.is_encrypted() && mib.object.id == MIID_UNKNOWN) {
      if (!listener->has_bindkey()) {
        MIOT_LOGW(TAG, "[%04X] Object is encrypted but bindkey is not configured", listener->get_address(),
                  listener->get_product_id());
        continue;
      }
      if (!decrypt_mibeacon(mib, listener->get_bindkey())) {
        continue;
      }
    }
    if (listener->process_mibeacon(mib)) {
      processed = true;
    }
  }

#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_DEBUG
  if (!processed) {
    ESP_LOGD(TAG, "Got MiBeacon: %s", format_hex_pretty(raw).c_str());
    const int rssi = device.get_rssi();
    ESP_LOGD(TAG, "  %s [%04X]%s %s RSSI=%d (%s)", device.get_name().c_str(), mib.product_id,
             mib.is_encrypted() ? " (encrypted)" : "", device.address_str().c_str(), rssi, get_signal_bars(rssi));
    if (mib.frame_control.solicited) {
      ESP_LOGD(TAG, "  Requesting for registration and binding");
    }
    if (mib.has_object()) {
      if (mib.is_encrypted() && mib.object.id == MIID_UNKNOWN) {
        ESP_LOGD(TAG, "  Data: %s", format_hex_pretty(mib.object.data).c_str());
      } else {
        ESP_LOGD(TAG, "  Object[%04X]: %s", mib.object.id, format_hex_pretty(mib.object.data).c_str());
      }
    }
  }
#endif

  return true;
}

bool MiotListener::process_mibeacon(const MiBeacon &mib) {
  if (this->product_id_ != 0 && this->product_id_ != mib.product_id) {
    return false;
  }

  if (this->frame_counter_ == mib.frame_counter) {
    MIOT_LOGV(TAG, "[%04X] Duplicate data packet received: %" PRIu8, this->address_, mib.product_id, mib.frame_counter);
    return true;
  }
  this->frame_counter_ = mib.frame_counter;

  if (!mib.has_object()) {
    return false;
  }

  MIOT_LOGV(TAG, "[%04X] Processing Object[%04X]: %s", this->address_, mib.product_id, mib.object.id,
            format_hex_pretty(mib.object.data).c_str());
  return this->process_object_(mib.object);
}

bool MiotListener::has_bindkey() const {
  for (uint8_t b : this->bindkey_) {
    if (b != 0) {
      return true;
    }
  }
  return false;
}

bool MiotListener::process_unhandled_(const miot::BLEObject &obj) {
  std::string s;
  if (obj.data.size() == 1) {
    s = str_sprintf("%u (0x%x)", obj.data[0], obj.data[0]);
  } else {
    s = format_hex_pretty(obj.data);
  }
  MIOT_LOGW(TAG, "[%04X] Unhandled object attribute: %04X, value: %s", this->address_, this->product_id_, obj.id,
            s.c_str());
  return false;
}

// Convert battery level to voltage.
static float battery_to_voltage(uint32_t battery_level) {
  // 2200 mV - 0%
  const uint32_t min_voltage = 2200;
  // 3100 mV - 100%
  const uint32_t max_voltage = 3100;
  return (min_voltage + (max_voltage - min_voltage) / 100 * battery_level) * 0.001f;
}

bool MiotComponent::process_default_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case MIID_BATTERY:
      if (this->battery_level_ || this->battery_voltage_) {
        auto battery_level = obj.get_battery_level();
        if (battery_level.has_value()) {
          if (this->battery_level_) {
            this->battery_level_->publish_state(*battery_level);
          }
          if (this->battery_voltage_) {
            this->battery_voltage_->publish_state(battery_to_voltage(*battery_level));
          }
        }
      }
      break;
    case MIID_MIAOMIAOCE_BATTERY_1003:
      if (this->battery_level_ || this->battery_voltage_) {
        auto battery_level = obj.get_miaomiaoce_battery_level_1003();
        if (battery_level.has_value()) {
          if (this->battery_level_) {
            this->battery_level_->publish_state(*battery_level);
          }
          if (this->battery_voltage_) {
            this->battery_voltage_->publish_state(battery_to_voltage(*battery_level));
          }
        }
      }
      break;
    default:
      return this->process_unhandled_(obj);
  }
  return true;
}

void MiotComponent::dump_config_(const char *TAG, const char *product_code) const {
  ESP_LOGCONFIG(TAG, "Xiaomi %s", product_code);
  const uint8_t *mac = mac_reverse(this->address_);
  ESP_LOGCONFIG(TAG, "  MAC: " MIOT_ADDR_STR, MIOT_ADDR_HEX_REVERSE(mac));
  if (this->has_bindkey()) {
    ESP_LOGCONFIG(TAG, "  Bindkey: %s", format_hex_pretty(this->bindkey_, sizeof(bindkey_t)).c_str());
  }
  LOG_SENSOR("  ", "Battery Level", this->battery_level_);
  LOG_SENSOR("  ", "Battery Voltage", this->battery_voltage_);
}

}  // namespace miot
}  // namespace esphome
