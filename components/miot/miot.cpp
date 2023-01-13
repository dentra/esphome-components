#include "esphome/core/log.h"
#include "mbedtls/ccm.h"
#include "inttypes.h"

#include "miot.h"
#include "miot_decrypt.h"

namespace esphome {
namespace miot {

static const char *const TAG = "miot";

// Unicode: U+2582, UTF-8: E2 96 82
#define UNI_LOWER_ONE_QUARTER_BLOCK "\xe2\x96\x82"
// Unicode: U+2584, UTF-8: E2 96 84
#define UNI_LOWER_HALF_BLOCK "\xe2\x96\x84"
// Unicode: U+2586, UTF-8: E2 96 86
#define UNI_LOWER_THREE_QUARTERS_BLOCK "\xe2\x96\x86"
// Unicode: U+2588, UTF-8: E2 96 88
#define UNI_FULL_BLOCK "\xe2\x96\x88"

const char *const get_signal_bars(int rssi) {
  if (rssi >= -50) {
    return ESPHOME_LOG_COLOR(ESPHOME_LOG_COLOR_GREEN) UNI_LOWER_ONE_QUARTER_BLOCK UNI_LOWER_HALF_BLOCK
        UNI_LOWER_THREE_QUARTERS_BLOCK UNI_FULL_BLOCK ESPHOME_LOG_RESET_COLOR;
  }
  if (rssi >= -65) {
    return ESPHOME_LOG_COLOR(ESPHOME_LOG_COLOR_YELLOW)
        UNI_LOWER_ONE_QUARTER_BLOCK UNI_LOWER_HALF_BLOCK UNI_LOWER_THREE_QUARTERS_BLOCK ESPHOME_LOG_COLOR(
            ESPHOME_LOG_COLOR_GRAY) UNI_FULL_BLOCK ESPHOME_LOG_RESET_COLOR;
  }
  if (rssi >= -85) {
    return ESPHOME_LOG_COLOR(ESPHOME_LOG_COLOR_YELLOW)
        UNI_LOWER_ONE_QUARTER_BLOCK UNI_LOWER_HALF_BLOCK ESPHOME_LOG_COLOR(ESPHOME_LOG_COLOR_GRAY)
            UNI_LOWER_THREE_QUARTERS_BLOCK UNI_FULL_BLOCK ESPHOME_LOG_RESET_COLOR;
  }
  return ESPHOME_LOG_COLOR(ESPHOME_LOG_COLOR_RED) UNI_LOWER_ONE_QUARTER_BLOCK ESPHOME_LOG_COLOR(ESPHOME_LOG_COLOR_GRAY)
      UNI_LOWER_HALF_BLOCK UNI_LOWER_THREE_QUARTERS_BLOCK UNI_FULL_BLOCK ESPHOME_LOG_RESET_COLOR;
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
    ESP_LOGW(TAG, "Invalid MiBeacon data length: %s", format_hex_pretty(raw).c_str());
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
  }

  if (mib.frame_control.capability_include) {
    mib.capability = *reinterpret_cast<const Capability *>(data);
    data = data + sizeof(Capability);
    if (mib.capability.io) {
      mib.io_capability = *reinterpret_cast<const IOCapability *>(data);
      data = data + sizeof(IOCapability);
    }
  }

  BLEObject *encrypted_obj = nullptr;
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
      encrypted_obj = new BLEObject(data, rnd);
    } else {
      mib.object = BLEObject((RawBLEObject *) data);
    }
  }

  bool processed = false;
  for (auto listener : this->listeners_) {
    if (device.address_uint64() != listener->get_address()) {
      continue;
    }
    if (mib.has_address() && mib.mac_address != device.address_uint64()) {
      ESP_LOGW(TAG, "%12" PRIX64 " [%04X] MAC address doesn't match data MAC address %12" PRIX64,
               listener->get_address(), listener->get_product_id(), mib.mac_address);
      continue;
    }
    if (encrypted_obj != nullptr) {
      if (!listener->have_bindkey()) {
        ESP_LOGW(TAG, "%12" PRIX64 " [%04X] Object is encrypted but bindkey is not configured", listener->get_address(),
                 listener->get_product_id());
        continue;
      }
      mib.object = *encrypted_obj;
      if (mib.frame_control.version > 3) {
        if (!decrypt_mibeacon45(listener, mib)) {
          continue;
        }
      } else {
        if (!decrypt_mibeacon23(listener, mib)) {
          continue;
        }
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
    ESP_LOGD(TAG, "  %s [%04X]%s %s RSSI=%d %s", device.get_name().c_str(), mib.product_id,
             mib.is_encrypted() ? " (encrypted)" : "", device.address_str().c_str(), rssi, get_signal_bars(rssi));
    if (mib.has_object()) {
      BLEObject *obj = nullptr;
      if (encrypted_obj != nullptr) {
        obj = encrypted_obj;
      } else if (!mib.is_encrypted()) {
        obj = &mib.object;
      }
      if (obj != nullptr) {
        ESP_LOGD(TAG, "  Object:");
        ESP_LOGD(TAG, "    ID  : %04X", obj->id);
        ESP_LOGD(TAG, "    data: %s", format_hex_pretty(obj->data.data(), obj->data.size()).c_str());
      }
    }
  }
#endif

  delete encrypted_obj;

  return true;
}

bool MiotListener::process_mibeacon(const MiBeacon &mib) {
  if (this->get_product_id() != mib.product_id) {
    return false;
  }

  if (this->frame_counter_ == mib.frame_counter) {
    ESP_LOGV(TAG, "%12" PRIX64 " [%04X] Duplicate data packet received: %" PRIu8, this->address_,
             this->get_product_id(), mib.frame_counter);
    return true;
  }
  this->frame_counter_ = mib.frame_counter;

  if (!mib.has_object()) {
    return false;
  }

  ESP_LOGV(TAG, "%12" PRIX64 " [%04X] Got BLEObject: ID: %04X, data: %s", this->address_, this->get_product_id(),
           mib.object.id, format_hex_pretty(mib.object.data).c_str());
  return this->process_object_(mib.object);
}

bool MiotListener::have_bindkey() const {
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
  ESP_LOGW(TAG, "%12" PRIX64 " [%04X] Unhandled object attribute: %04X, value: %s", this->address_,
           this->get_product_id(), obj.id, s.c_str());
  return false;
}

// Convert battery level to voltage.
float battery_to_voltage(uint32_t battery_level) {
  // 2200 mV - 0%
  const uint32_t min_voltage = 2200;
  // 3100 mV - 100%
  const uint32_t max_voltage = 3100;
  return (min_voltage + (max_voltage - min_voltage) / 100 * battery_level) * 0.001f;
}

bool MiotComponent::process_default_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case MIID_BATTERY: {
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
    }
    case MIID_MIAOMIAOCE_BATTERY: {
      if (this->battery_level_ || this->battery_voltage_) {
        auto battery_level = obj.get_miaomiaoce_battery_level();
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
    }
    default:
      return this->process_unhandled_(obj);
  }
  return true;
}

void MiotComponent::dump_config_(const char *TAG) const {
  ESP_LOGCONFIG(TAG, "Xiaomi %s", this->get_product_code());
  const uint8_t *mac = mac_reverse(this->address_);
  ESP_LOGCONFIG(TAG, "  MAC: " MIOT_ADDR_STR, MIOT_ADDR_HEX_REVERSE(mac));
  if (this->have_bindkey()) {
    ESP_LOGCONFIG(TAG, "  Bindkey: %s", format_hex_pretty(this->bindkey_, sizeof(bindkey_t)).c_str());
  }
  LOG_SENSOR("  ", "Battery Level", this->battery_level_);
  LOG_SENSOR("  ", "Battery Voltage", this->battery_voltage_);
}

}  // namespace miot
}  // namespace esphome
