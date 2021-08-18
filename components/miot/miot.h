#pragma once
#ifdef ARDUINO_ARCH_ESP32
#include <functional>
#include <vector>
#include "esphome/core/component.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"
#include "miot_spec.h"

namespace esphome {
namespace miot {

using bindkey_t = uint8_t[16];

struct BLEObject {
  BLEObject() : id(MIID_UNINITIALIZED){};
  BLEObject(RawBLEObject *obj) : id(obj->id), data(obj->data, obj->data + obj->data_len) {}

  MIID id;
  std::vector<uint8_t> data;

  // get 8 bit value as boolean.
  optional<bool> get_bool() const { return get_uint8(); }
  // get 8 bit unsigned value.
  optional<uint8_t> get_uint8() const {
    if (data.size() == sizeof(uint8_t)) {
      return data[0];
    }
    return {};
  }
  // get 16 bit signed value.
  optional<int16_t> get_int16() const {
    if (data.size() == sizeof(int16_t)) {
      return *reinterpret_cast<const int16_t *>(data.data());
    }
    return {};
  }
  // get 16 bit unsigned value.
  optional<uint16_t> get_uint16() const {
    if (data.size() == sizeof(uint16_t)) {
      return *reinterpret_cast<const uint16_t *>(data.data());
    }
    return {};
  }
  // get 24 bit unsigned values.
  optional<uint32_t> get_uint24() const {
    if (data.size() == sizeof(uint32_t) - 1) {
      return (*reinterpret_cast<const uint32_t *>(data.data())) & 0x00FFFFFF;
    }
    return {};
  }
  // get 32 or 24 bit unsigned values.
  optional<uint32_t> get_uint32() const {
    if (data.size() == sizeof(uint32_t)) {
      return *reinterpret_cast<const uint32_t *>(data.data());
    }
    return {};
  }
  // get typed value. example: obj.get_typed<MyData>();
  template<typename T> optional<const T *> get_typed() const {
    if (data.size() == sizeof(T)) {
      return reinterpret_cast<const T *>(data.data());
    }
    return {};
  }
  // Value for MIID_DOOR_SENSOR.
  // See MIID_DOOR_SENSOR for detailed description for returning value.
  optional<uint8_t> get_door_sensor() const;
  // Value for MIID_IDLE_TIME.
  // returns idle time in seconds.
  optional<uint32_t> get_idle_time() const;
  // Value for MIID_TIMEOUT.
  // returns timeout in seconds.
  optional<uint32_t> get_timeout() const;
  // Value for MIID_MOTION_WITH_LIGHT_EVENT.
  // returns illuminance in lux.
  optional<uint32_t> get_motion_with_light_event() const;
  // Value for MIID_FLOODING.
  // returns true on flooding or false if not.
  optional<bool> get_flooding() const;
  // Value for MIID_LIGHT_INTENSITY.
  // returns true for a strong and false for a weak light.
  optional<bool> get_light_intensity() const;
  // Value for MIID_TEMPERATURE.
  // returns temperature in °C.
  optional<float> get_temperature() const;
  // Value for MIID_HUMIDITY.
  // returns humidity in %.
  optional<float> get_humidity() const;
  // Value for MIID_TEMPERATURE_HUMIDITY.
  // returns TemperatureHumidity object.
  optional<const TemperatureHumidity> get_temperature_humidity() const;
  // Value for MIID_BUTTON_EVENT.
  // returns ButtonEvent object.
  optional<const ButtonEvent> get_button_event() const;
};

struct MiBeacon {
  MiBeacon() : frame_control({}), product_id(0), frame_counter(0){};
  MiBeacon(const RawMiBeaconHeader *hdr)
      : frame_control(hdr->frame_control), product_id(hdr->product_id), frame_counter(hdr->frame_counter) {}
  FrameControl frame_control;
  uint16_t product_id;
  uint8_t frame_counter;
  uint64_t mac_address = {};
  Capability capability = {};
  IOCapability io_capability = {};
  BLEObject object = {};
  uint32_t random_number = {};
  uint32_t message_integrity_check = {};
};

class MiotListener : public esp32_ble_tracker::ESPBTDeviceListener {
 public:
  void set_address(uint64_t address) { address_ = address; };
  void set_bindkey(const bindkey_t bindkey) { memcpy(this->bindkey_, bindkey, sizeof(bindkey_t)); }

  virtual uint16_t get_product_id() const = 0;

  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;

 protected:
  uint64_t address_ = {};
  bindkey_t bindkey_ = {};
  uint8_t last_frame_counter_ = {};

  virtual void process_object_(const BLEObject &obj) = 0;
  virtual bool process_mibeacon_(const MiBeacon &mib);

  optional<MiBeacon> parse_mibeacon_(const std::vector<uint8_t> &raw);
  optional<BLEObject> decrypt_mibeacon_(const MiBeacon &mib, const uint8_t *data, size_t size);
};

class MiotComponent : public Component, public MiotListener {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }

  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;

  virtual const char *get_product_code() const = 0;
  void set_battery_level(sensor::Sensor *battery_level) { this->battery_level_ = battery_level; }
  void set_rssi(sensor::Sensor *rssi) { this->rssi_ = rssi; }

 protected:
  sensor::Sensor *battery_level_{nullptr};
  sensor::Sensor *rssi_{nullptr};

  void dump_config_(const char *TAG) const;

  void process_default_(const miot::BLEObject &obj);
};

}  // namespace miot
}  // namespace esphome
#endif
