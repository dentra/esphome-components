#ifdef ARDUINO_ARCH_ESP32
#include "esphome/core/log.h"
#include "mbedtls/ccm.h"
#include "inttypes.h"

#include "miot.h"

namespace esphome {
namespace miot {

static const char *const TAG = "miot";

#define _MIOT_LOG(logger, format, ...) \
  logger(TAG, "%12" PRIX64 " [%04X] " format, this->address_, this->get_product_id(), ##__VA_ARGS__)

#define MIOT_LOGW(format, ...) _MIOT_LOG(ESP_LOGW, format, ##__VA_ARGS__)
#define MIOT_LOGD(format, ...) _MIOT_LOG(ESP_LOGD, format, ##__VA_ARGS__)
#define MIOT_LOGV(format, ...) _MIOT_LOG(ESP_LOGV, format, ##__VA_ARGS__)

optional<BLEObject> MiotListener::decrypt_mibeacon_(const MiBeacon &mib, const uint8_t *data, size_t size) {
  if (size > sizeof(RawBLEObject)) {
    MIOT_LOGW("Encryped data is too large");
    return {};
  }

  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);

  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, this->bindkey_, sizeof(bindkey_t) * 8);  // 8 bits per byte
  if (ret != 0) {
    MIOT_LOGW("mbedtls_ccm_setkey failed: %d", ret);
    mbedtls_ccm_free(&ctx);
    return {};
  }

  struct {
    esp_bd_addr_t mac;
    uint16_t product_id;
    uint32_t random_number;
  } PACKED nonce;
  memcpy(nonce.mac, &this->address_, sizeof(esp_bd_addr_t));
  nonce.product_id = mib.product_id;
  nonce.random_number = mib.random_number;

  const uint8_t *iv = reinterpret_cast<const uint8_t *>(&nonce);
  const size_t iv_len = sizeof(nonce);
  const uint8_t add[] = {0x11};
  const size_t add_len = sizeof(add);
  const uint8_t *tag = reinterpret_cast<const uint8_t *>(&mib.message_integrity_check);
  const size_t tag_len = sizeof(mib.message_integrity_check);

  uint8_t result[sizeof(RawBLEObject)];
  ret = mbedtls_ccm_auth_decrypt(&ctx, size, iv, iv_len, add, add_len, data, result, tag, tag_len);
  mbedtls_ccm_free(&ctx);
  if (ret != 0) {
    MIOT_LOGW("mbedtls_ccm_auth_decrypt failed: %d", ret);
    ESP_LOGW(TAG, "   mac: %02X:%02X:%02X:%02X:%02X:%02X", nonce.mac[5], nonce.mac[4], nonce.mac[3], nonce.mac[2],
             nonce.mac[1], nonce.mac[0]);
    ESP_LOGW(TAG, "   key: %s", hexencode(this->bindkey_, sizeof(bindkey_t)).c_str());
    ESP_LOGW(TAG, "    lv: %s", hexencode(iv, iv_len).c_str());
    ESP_LOGW(TAG, "   add: %s", hexencode(add, add_len).c_str());
    ESP_LOGW(TAG, "   tag: %s", hexencode(tag, tag_len).c_str());
    ESP_LOGW(TAG, "  data: %s", hexencode(data, size).c_str());
    return {};
  }

  return BLEObject(reinterpret_cast<RawBLEObject *>(result));
}

optional<MiBeacon> MiotListener::parse_mibeacon_(const std::vector<uint8_t> &raw) {
  if (raw.size() < sizeof(RawMiBeaconHeader)) {
    MIOT_LOGW("Invalid MiBeacon data length: %s", hexencode(raw).c_str());
    return {};
  }

  auto mib = MiBeacon(reinterpret_cast<const RawMiBeaconHeader *>(raw.data()));

  if (this->get_product_id() != 0 && this->get_product_id() != mib.product_id) {
    MIOT_LOGW("Product ID does't match data product ID %04X", mib.product_id);
    return {};
  }

  if (mib.frame_control.mesh) {
    MIOT_LOGW("Device data is a mesh type device");
    return {};
  }

  if (mib.frame_control.version < 2) {
    MIOT_LOGW("Device data is using old data format: %" PRIu8, mib.frame_control.version);
    return {};
  }

  if (this->last_frame_counter_ == mib.frame_counter) {
    MIOT_LOGV("Duplicate data packet received: %" PRIu8, this->last_frame_counter_);
    return {};
  }
  this->last_frame_counter_ = mib.frame_counter;

  auto data = raw.data() + sizeof(RawMiBeaconHeader);

  if (mib.frame_control.mac_include) {
    mib.mac_address =
        (*reinterpret_cast<const uint64_t *>(data)) & 0x0000FFFFFFFFFFFFul;  // mac address 6 bytes + 2 bytes for uint64
    data = data + sizeof(esp_bd_addr_t);
    if (mib.mac_address != this->address_) {
      MIOT_LOGW("MAC address doesn't match data MAC address %12" PRIX64, mib.mac_address);
      return {};
    }
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
      if (!this->have_bindkey()) {
        MIOT_LOGW("Object is encryped but bindkey is not configured");
        return {};
      }
      const uint8_t *mic = raw.data() + raw.size() - sizeof(uint32_t);
      mib.message_integrity_check = *reinterpret_cast<const uint32_t *>(mic);

      const uint8_t *rnd = mic - sizeof(uint32_t);
      // random_number combined with frame_counter to become a 4-byte Counter for anti-replay
      mib.random_number = ((*reinterpret_cast<const uint32_t *>(rnd)) & 0xFFFFFF00) | mib.frame_counter;
      if (mib.frame_control.version > 3) {
        // random number size is 3 bytes length, so add one.
        auto obj = this->decrypt_mibeacon_(mib, data, rnd + 1 - data);
        if (!obj.has_value()) {
          MIOT_LOGW("Decryption failed");
          return {};
        }
        mib.object = *obj;
      } else {
        MIOT_LOGW("Can't decrypt object of version %" PRIu8, mib.frame_control.version);
        return {};
      }
    } else {
      mib.object = BLEObject((RawBLEObject *) data);
    }
  }

  return mib;
}

bool MiotListener::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  if (device.address_uint64() != this->address_) {
    MIOT_LOGV("Unknown MAC address");
    return false;
  }
  MIOT_LOGV("MAC address found: %s", device.address_str().c_str());

  for (auto &service_data : device.get_service_datas()) {
    if (!service_data.uuid.contains(0x95, 0xFE)) {
      MIOT_LOGV("No Xiaomi ServiceData UUID (FE95) found");
      continue;
    }
    auto mib = this->parse_mibeacon_(service_data.data);
    if (mib.has_value()) {
      MIOT_LOGD("Got MiBeacon: %s", hexencode(service_data.data).c_str());
      if (this->process_mibeacon_(*mib)) {
        return true;
      }
    }
  }

  return false;
}

bool MiotListener::process_mibeacon_(const MiBeacon &mib) {
  if (!mib.frame_control.object_include) {
    return false;
  }
  MIOT_LOGD("Got BLEObject: ID: %04X, data: %s", mib.object.id, hexencode(mib.object.data).c_str());
  return this->process_object_(mib.object);
}

bool MiotListener::process_unhandled_(const miot::BLEObject &obj) {
  MIOT_LOGW("Unhandled object attribute: %04X, value: %s", obj.id, hexencode(obj.data).c_str());
  return false;
}

bool MiotListener::have_bindkey() const {
  for (uint8_t b : this->bindkey_) {
    if (b != 0) {
      return true;
    }
  }
  return false;
}

bool MiotComponent::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  if (!MiotListener::parse_device(device)) {
    return false;
  }
  if (this->rssi_ != nullptr) {
    this->rssi_->publish_state(device.get_rssi());
  }
  return true;
}

bool MiotComponent::process_default_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case MIID_BATTERY: {
      if (this->battery_level_) {
        auto battery_level = obj.get_battery_level();
        if (battery_level.has_value()) {
          this->battery_level_->publish_state(*battery_level);
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
  const uint8_t *mac = reinterpret_cast<const uint8_t *>(&this->address_);
  ESP_LOGCONFIG(TAG, "  MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  if (this->have_bindkey()) {
    ESP_LOGCONFIG(TAG, "  Bindkey: %s", hexencode(this->bindkey_, sizeof(bindkey_t)).c_str());
  }
  LOG_SENSOR("  ", "Battery Level", this->battery_level_);
  LOG_SENSOR("  ", "RSSI", this->rssi_);
}

}  // namespace miot
}  // namespace esphome
#endif
