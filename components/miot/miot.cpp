#ifdef ARDUINO_ARCH_ESP32
#include "esphome/core/log.h"
#include "mbedtls/ccm.h"
#include "inttypes.h"

#include "miot.h"

namespace esphome {
namespace miot {

static const char *TAG = "miot";

optional<BLEObject> MiotListener::decrypt_mibeacon_(const MiBeacon &mib, const uint8_t *data, size_t size) {
  if (size > sizeof(RawBLEObject)) {
    ESP_LOGW(TAG, "Encryped data is too large");
    return {};
  }

  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);

  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, this->bindkey_, sizeof(bindkey_t) * 8);  // 8 bits per byte
  if (ret != 0) {
    ESP_LOGW(TAG, "mbedtls_ccm_setkey failed: %d", ret);
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
    ESP_LOGW(TAG, "mbedtls_ccm_auth_decrypt failed: %d", ret);
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
    ESP_LOGW(TAG, "Invalid MiBeacon data length: %s", hexencode(raw.data(), raw.size()).c_str());
    return {};
  }

  auto mib = MiBeacon(reinterpret_cast<const RawMiBeaconHeader *>(raw.data()));

  if (this->get_product_id() != 0 && this->get_product_id() != mib.product_id) {
    const uint8_t *mac = reinterpret_cast<const uint8_t *>(&this->address_);
    ESP_LOGW(TAG, "Product ID (%04X) does't mathch data product ID %04X, MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             this->get_product_id(), mib.product_id, mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    return {};
  }

  if (mib.frame_control.mesh) {
    ESP_LOGW(TAG, "Device data is a mesh type device: %s", hexencode(raw.data(), raw.size()).c_str());
    return {};
  }

  if (mib.frame_control.version < 2) {
    ESP_LOGW(TAG, "Device data is using old data format: %s", hexencode(raw.data(), raw.size()).c_str());
    return {};
  }

  if (this->last_frame_counter_ == mib.frame_counter) {
    ESP_LOGVV(TAG, "Duplicate data packet received (%" PRIu8 ")", this->last_frame_counter_);
    return {};
  }
  this->last_frame_counter_ = mib.frame_counter;

  auto data = raw.data() + sizeof(RawMiBeaconHeader);

  if (mib.frame_control.mac_include) {
    mib.mac_address =
        (*reinterpret_cast<const uint64_t *>(data)) & 0x0000FFFFFFFFFFFFul;  // mac address 6 bytes + 2 bytes for uint64
    data = data + sizeof(esp_bd_addr_t);
    if (mib.mac_address != this->address_) {
      ESP_LOGW(TAG, "MAC address (%12" PRIx64 ") doesn't match data MAC address %12" PRIx64, this->address_,
               mib.mac_address);
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
      const uint8_t *mic = raw.data() + raw.size() - sizeof(uint32_t);
      mib.message_integrity_check = *reinterpret_cast<const uint32_t *>(mic);

      const uint8_t *rnd = mic - sizeof(uint32_t);
      // random_number combined with frame_counter to become a 4-byte Counter for anti-replay
      mib.random_number = ((*reinterpret_cast<const uint32_t *>(rnd)) & 0xFFFFFF00) | mib.frame_counter;
      if (mib.frame_control.version > 3) {
        // random number size is 3 bytes length, so add one.
        auto obj = this->decrypt_mibeacon_(mib, data, rnd + 1 - data);
        if (!obj.has_value()) {
          ESP_LOGW(TAG, "Decryption failed %s", hexencode(raw.data(), raw.size()).c_str());
          return {};
        }
        mib.object = *obj;
      } else {
        ESP_LOGW(TAG, "Can't decrypt object of version %" PRIu8, mib.frame_control.version);
      }
    } else {
      mib.object = BLEObject((RawBLEObject *) data);
    }
  }

  return mib;
}

bool MiotListener::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  if (device.address_uint64() != this->address_) {
    ESP_LOGVV(TAG, "Unknown MAC address.");
    return false;
  }
  ESP_LOGVV(TAG, "MAC address %s found.", device.address_str().c_str());

  for (auto &service_data : device.get_service_datas()) {
    if (!service_data.uuid.contains(0x95, 0xFE)) {
      ESP_LOGVV(TAG, "No service data UUID magic bytes");
      continue;
    }
    auto mib = this->parse_mibeacon_(service_data.data);
    if (mib.has_value()) {
      ESP_LOGD(TAG, "[%04X] got MiBeacon: %s", this->get_product_id(),
               hexencode(service_data.data.data(), service_data.data.size()).c_str());
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
  ESP_LOGI(TAG, "[%04X] got object: ID: %04X, data: %s", this->get_product_id(), mib.object.id,
           hexencode(mib.object.data.data(), mib.object.data.size()).c_str());
  this->process_object_(mib.object);
  return true;
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

bool MiotComponent::process_mibeacon_(const MiBeacon &mib) {
  if (!MiotListener::process_mibeacon_(mib)) {
    return false;
  }
  if (this->battery_level_) {
    auto battery_level = mib.object.get_uint8(ATTR_BATTERY);
    if (battery_level.has_value()) {
      this->battery_level_->publish_state(*battery_level);
    }
  }
  return true;
}

void MiotComponent::dump_config_(const char *TAG) const {
  ESP_LOGCONFIG(TAG, "Xiaomi %s", this->get_product_code());
  const uint8_t *mac = reinterpret_cast<const uint8_t *>(&this->address_);
  ESP_LOGCONFIG(TAG, "  MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  for (uint8_t b : this->bindkey_) {
    if (b != 0) {
      ESP_LOGCONFIG(TAG, "  Bindkey: %s", hexencode(this->bindkey_, sizeof(bindkey_t)).c_str());
      break;
    }
  }
  LOG_SENSOR("  ", "Battery Level", this->battery_level_);
  LOG_SENSOR("  ", "RSSI", this->rssi_);
}

}  // namespace miot
}  // namespace esphome
#endif
