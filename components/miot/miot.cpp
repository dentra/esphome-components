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

void MiotComponent::process_default_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case MIID_BATTERY: {
      if (this->battery_level_) {
        auto battery_level = obj.get_uint8();
        if (battery_level.has_value()) {
          this->battery_level_->publish_state(*battery_level);
        }
      }
      break;
    }

    default:
      ESP_LOGD(TAG, "[%04X] Unhandled object attribute: %04X, value: %s", this->get_product_id(), obj.id,
               hexencode(obj.data).c_str());
      break;
  }
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

#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_DEBUG
#define CHECK_MIID(miid) \
  if (this->id != miid) { \
    ESP_LOGW(TAG, "Invalid " #miid " call"); \
    return {}; \
  }
#else
#define CHECK_MIID(miid)
#endif

optional<uint8_t> BLEObject::get_door_sensor() const {
  CHECK_MIID(MIID_DOOR_SENSOR);
  const auto opening = this->get_uint8();
  if (opening.has_value()) {
    ESP_LOGD(TAG, "Opening: %" PRIu8, *opening);
  }
  return opening;
}

optional<uint32_t> BLEObject::get_idle_time() const {
  CHECK_MIID(MIID_IDLE_TIME);
  const auto idle_time = this->get_uint32();
  if (idle_time.has_value()) {
    ESP_LOGD(TAG, "Idle time: %u s", *idle_time);
  }
  return idle_time;
}

optional<uint32_t> BLEObject::get_timeout() const {
  CHECK_MIID(MIID_TIMEOUT);
  const auto timeout = this->get_uint32();
  if (timeout.has_value()) {
    ESP_LOGD(TAG, "Timeout: %u s", *timeout);
  }
  return timeout;
}

optional<uint32_t> BLEObject::get_motion_with_light_event() const {
  CHECK_MIID(MIID_MOTION_WITH_LIGHT_EVENT);
  const auto illuminance = this->get_uint24();
  if (illuminance.has_value()) {
    ESP_LOGD(TAG, "Motion with light: %u lx", *illuminance);
  }
  return illuminance;
}

optional<bool> BLEObject::get_flooding() const {
  CHECK_MIID(MIID_FLOODING);
  const auto flooding = this->get_bool();
  if (flooding.has_value()) {
    ESP_LOGD(TAG, "Flooding: %s", YESNO(*flooding));
  }
  return flooding;
}

optional<bool> BLEObject::get_light_intensity() const {
  CHECK_MIID(MIID_LIGHT_INTENSITY);
  const auto light = this->get_bool();
  if (light.has_value()) {
    ESP_LOGD(TAG, "Light intensity: %s", *light ? "strong" : "weak");
  }
  return light;
}

optional<float> BLEObject::get_temperature() const {
  CHECK_MIID(MIID_TEMPERATURE);
  const auto temperature = this->get_int16();
  if (!temperature.has_value()) {
    return temperature;
  }
  const float res = *temperature * 0.1f;
  ESP_LOGD(TAG, "Temperature %.1f °C", res);
  return res;
}

optional<float> BLEObject::get_humidity() const {
  CHECK_MIID(MIID_HUMIDITY);
  const auto humidity = this->get_uint16();
  if (!humidity.has_value()) {
    return humidity;
  }
  const float res = *humidity * 0.1f;
  ESP_LOGD(TAG, "Humidity %.1f %%", res);
  return res;
}

optional<const TemperatureHumidity> BLEObject::get_temperature_humidity() const {
  CHECK_MIID(MIID_TEMPERATURE_HUMIDITY);
  struct _TemperatureHumidity {
    uint16_t temperature;
    uint16_t humidity;
  };
  const auto typed = this->get_typed<_TemperatureHumidity>();
  if (!typed.has_value()) {
    return {};
  }
  TemperatureHumidity res;
  res.temperature = (*typed)->temperature * 0.1f;
  ESP_LOGD(TAG, "Temperature %.1f °C", res.temperature);
  res.humidity = (*typed)->humidity * 0.1f;
  ESP_LOGD(TAG, "Humidity %.1f %%", res.humidity);
  return res;
}

optional<const ButtonEvent> BLEObject::get_button_event() const {
  CHECK_MIID(MIID_BUTTON_EVENT);
  const auto button_event = this->get_typed<ButtonEvent>();
  if (!button_event.has_value()) {
    return {};
  }
  const auto &res = *(*button_event);
  switch (res.type) {
    case ButtonEvent::CLICK:
      ESP_LOGD(TAG, "Button click: %u", res.index);
      break;
    case ButtonEvent::DOUBLE_CLICK:
      ESP_LOGD(TAG, "Button double click: %u", res.index);
      break;
    case ButtonEvent::TRIPLE_CLICK:
      ESP_LOGD(TAG, "Button triple click: %u", res.index);
      break;
    case ButtonEvent::LONG_PRESS:
      ESP_LOGD(TAG, "Button long press: %u", res.index);
      break;
    default:
      ESP_LOGD(TAG, "Button unknown event %02" PRIx8 ": %u", res.type, res.index);
      break;
  }
  return res;
}

}  // namespace miot
}  // namespace esphome
#endif
