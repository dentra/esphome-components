#ifdef ARDUINO_ARCH_ESP32
#include "esphome/core/log.h"
#include "mbedtls/ccm.h"
#include "inttypes.h"

#include "miot.h"

namespace esphome {
namespace miot {

static const char *const TAG = "miot";

bool MiBeaconTracker::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  bool processed = false;
  for (auto &service_data : device.get_service_datas()) {
    if (service_data.uuid.contains(0x95, 0xFE)) {
      return this->parse_mibeacon_(device.address_uint64(), service_data.data);
    }
  }
  return false;
}

bool MiBeaconTracker::decrypt_mibeacon2_(MiotListener *listener, MiBeacon &mib) const {
  ESP_LOGW(TAG, "%12" PRIX64 " [%04X] Can't decrypt object of version %" PRIu8, listener->get_address(),
           listener->get_product_id(), mib.frame_control.version);
  return false;
}

bool MiBeaconTracker::decrypt_mibeacon4_(MiotListener *listener, MiBeacon &mib) const {
  const uint8_t *data = mib.object.data.data();
  size_t size = mib.object.data.size();
  if (size > sizeof(RawBLEObject)) {
    ESP_LOGW(TAG, "%12" PRIX64 " [%04X] Encryped data is too large", listener->get_address(),
             listener->get_product_id());
    return false;
  }

  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);

  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, listener->get_bindkey(),
                               sizeof(bindkey_t) * 8);  // 8 bits per byte
  if (ret != 0) {
    ESP_LOGW(TAG, "%12" PRIX64 " [%04X] mbedtls_ccm_setkey failed: %d", listener->get_address(),
             listener->get_product_id(), ret);
    mbedtls_ccm_free(&ctx);
    return false;
  }

  struct {
    esp_bd_addr_t mac;
    uint16_t product_id;
    uint32_t random_number;
  } PACKED nonce;
  memcpy(nonce.mac, &listener->get_address(), sizeof(esp_bd_addr_t));
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
    ESP_LOGW(TAG, "%12" PRIX64 " [%04X] mbedtls_ccm_auth_decrypt failed: %d", listener->get_address(),
             listener->get_product_id(), ret);
    ESP_LOGW(TAG, "   mac: %02X:%02X:%02X:%02X:%02X:%02X", nonce.mac[5], nonce.mac[4], nonce.mac[3], nonce.mac[2],
             nonce.mac[1], nonce.mac[0]);
    ESP_LOGW(TAG, "   key: %s", hexencode(listener->get_bindkey(), sizeof(bindkey_t)).c_str());
    ESP_LOGW(TAG, "    iv: %s", hexencode(iv, iv_len).c_str());
    ESP_LOGW(TAG, "   add: %s", hexencode(add, add_len).c_str());
    ESP_LOGW(TAG, "   tag: %s", hexencode(tag, tag_len).c_str());
    ESP_LOGW(TAG, "  data: %s", hexencode(data, size).c_str());
    return false;
  }

  mib.object = BLEObject(reinterpret_cast<RawBLEObject *>(result));

  return true;
}

bool MiBeaconTracker::parse_mibeacon_(const uint64_t &address, const std::vector<uint8_t> &raw) const {
  if (raw.size() < sizeof(RawMiBeaconHeader)) {
    ESP_LOGW("Invalid MiBeacon data length: %s", hexencode(raw).c_str());
    return false;
  }

  ESP_LOGD(TAG, "Got MiBeacon: %s", hexencode(raw).c_str());

  auto mib = MiBeacon(reinterpret_cast<const RawMiBeaconHeader *>(raw.data()));

  if (mib.frame_control.mesh) {
    ESP_LOGW(TAG, "Device data is a mesh type device");
    return false;
  }

  if (mib.frame_control.version < 2) {
    ESP_LOGW(TAG, "Device data is using old data format: %" PRIu8, mib.frame_control.version);
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

  BLEObject *encryped_obj = nullptr;
  if (mib.frame_control.object_include) {
    if (mib.frame_control.is_encrypted) {
      const uint8_t *mic = raw.data() + raw.size() - sizeof(uint32_t);
      mib.message_integrity_check = *reinterpret_cast<const uint32_t *>(mic);

      const uint8_t *rnd = mic - sizeof(uint32_t);
      // random_number combined with frame_counter to become a 4-byte Counter for anti-replay
      mib.random_number = ((*reinterpret_cast<const uint32_t *>(rnd)) & 0xFFFFFF00) | mib.frame_counter;
      rnd++;  // random number size is 3 bytes length, so add one.
      encryped_obj = new BLEObject(data, rnd);
    } else {
      mib.object = BLEObject((RawBLEObject *) data);
    }
  }

  bool processed = false;
  for (auto listener : listeners_) {
    if (address != listener->get_address()) {
      continue;
    }
    if (mib.frame_control.mac_include && address != mib.mac_address) {
      ESP_LOGW(TAG, "%12" PRIX64 " [%04X] MAC address doesn't match data MAC address %12" PRIX64,
               listener->get_address(), listener->get_product_id(), mib.mac_address);
      continue;
    }
    if (encryped_obj != nullptr) {
      if (!listener->have_bindkey()) {
        ESP_LOGW(TAG, "%12" PRIX64 " [%04X] Object is encryped but bindkey is not configured", listener->get_address(),
                 listener->get_product_id());
        continue;
      }
      mib.object = *encryped_obj;
      if (mib.frame_control.version > 3) {
        if (!this->decrypt_mibeacon4_(listener, mib)) {
          continue;
        }
      } else {
        if (!this->decrypt_mibeacon2_(listener, mib)) {
          continue;
        }
      }
    }
    if (listener->process_mibeacon(mib)) {
      processed = true;
    }
  }

  delete encryped_obj;

  return processed;
}

bool MiotListener::process_mibeacon(const MiBeacon &mib) {
  if (this->get_product_id() != mib.product_id) {
    return false;
  }

  if (this->frame_counter_ == mib.frame_counter) {
    ESP_LOGD(TAG, "%12" PRIX64 " [%04X] Duplicate data packet received: %" PRIu8, this->address_,
             this->get_product_id(), mib.frame_counter);
    return true;
  }
  this->frame_counter_ = mib.frame_counter;

  if (!mib.frame_control.object_include) {
    return false;
  }

  ESP_LOGD(TAG, "%12" PRIX64 " [%04X] Got BLEObject: ID: %04X, data: %s", this->address_, this->get_product_id(),
           mib.object.id, hexencode(mib.object.data).c_str());
  return this->process_object_(mib.object);
}

bool MiotListener::process_unhandled_(const miot::BLEObject &obj) {
  ESP_LOGW(TAG, "%12" PRIX64 " [%04X] Unhandled object attribute: %04X, value: %s", this->address_,
           this->get_product_id(), obj.id, hexencode(obj.data).c_str());
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
}

}  // namespace miot
}  // namespace esphome
#endif
