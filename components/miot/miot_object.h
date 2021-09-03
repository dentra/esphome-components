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

// Object returned by MIID_TEMPERATURE_HUMIDITY.
struct TemperatureHumidity {
  float temperature = {};
  float humidity = {};
};

// Object returned by MIID_BUTTON_EVENT.
struct ButtonEvent {
  // Button number, value range 0~9.
  uint16_t index = 0xFFFF;
  // Type of event.
  enum Type : uint8_t {
    CLICK = 0x00,
    DOUBLE_CLICK = 0x01,
    LONG_PRESS = 0x02,
    TRIPLE_CLICK = 0x03,
    _UNINITIALIZED = 0xFF,
  } type = _UNINITIALIZED;
} PACKED;

struct BLEObject {
  BLEObject() : id(MIID_UNINITIALIZED) {}
  BLEObject(const uint8_t *encrypted_begin, const uint8_t *encrypted_end)
      : id(MIID_UNINITIALIZED), data(encrypted_begin, encrypted_end) {}
  BLEObject(RawBLEObject *obj) : id(obj->id), data(obj->data, obj->data + obj->data_len) {}

  MIID id;
  // holds encrypted data when id==MIID_UNINITIALIZED
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
  /**
   * Value for MIID_BATTERY.
   * @return battery level in %.
   */
  optional<uint8_t> get_battery_level() const;
  /**
   * Value for MIID_DOOR_SENSOR.
   * @return See MIID_DOOR_SENSOR for detailed description for returning value.
   */
  optional<uint8_t> get_door_sensor() const;
  /**
   * Value for MIID_IDLE_TIME.
   * @return idle time in seconds.
   */
  optional<uint32_t> get_idle_time() const;
  /**
   * Value for MIID_TIMEOUT.
   * @return timeout in seconds.
   */
  optional<uint32_t> get_timeout() const;
  /**
   * Value for MIID_MOTION_WITH_LIGHT_EVENT.
   * @return illuminance in lux.
   */
  optional<uint32_t> get_motion_with_light_event() const;
  /**
   * Value for MIID_FLOODING.
   * @return true on flooding or false if not.
   */
  optional<bool> get_flooding() const;
  /**
   * Value for MIID_LIGHT_INTENSITY.
   * @return true for a strong and false for a weak light.
   */
  optional<bool> get_light_intensity() const;
  /**
   * Value for MIID_TEMPERATURE.
   * @return temperature in °C.
   */
  optional<float> get_temperature() const;
  /**
   * Value for MIID_HUMIDITY.
   * @return humidity in %.
   */
  optional<float> get_humidity() const;
  /**
   * Value for MIID_TEMPERATURE_HUMIDITY.
   * @return TemperatureHumidity object.
   */
  optional<const TemperatureHumidity> get_temperature_humidity() const;
  /**
   * Value for MIID_BUTTON_EVENT.
   * @return ButtonEvent object.
   */
  optional<const ButtonEvent> get_button_event() const;
  /**
   * Value for MIID_ILLUMINANCE.
   * @return illuminance in lux, range 0-120000.
   */
  optional<float> get_illuminance() const;
};

}  // namespace miot
}  // namespace esphome
#endif
