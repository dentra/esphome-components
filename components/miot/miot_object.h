#pragma once

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
  uint16_t temperature;
  uint16_t humidity;

  // temperature in °C with 1 accuracy decimals
  float get_temperature() const { return temperature * .1f; }
  // humidity in % with 1 accuracy decimals
  float get_humidity() const { return humidity * .1f; }
};

// Object returned by MIID_BUTTON_EVENT.
struct ButtonEvent {
  // Button number, value range 0~9.
  union {
    struct {
      uint16_t index;
    } button;
    struct {
      uint8_t index;
      uint8_t value;
      uint8_t short_press() const { return index == 0 ? value : 0; }
      uint8_t long_press() const { return index != 0 ? index : 0; }
    } knob;
    struct {
      int8_t index;
      int8_t value;
      // get rotate position if rotated left or 0 otherwise
      uint8_t left() const { return index == 0 && value < 0 ? -value : 0; }
      // get rotate position if rotated right or 0 otherwise
      uint8_t right() const { return index == 0 && value > 0 ? value : 0; }
      // get rotate position if rotated left with or 0 otherwise
      uint8_t left_pressed() const { return index != 0 && index < 0 ? -index : 0; }
      // get rotate position if rotated right with or 0 otherwise
      uint8_t right_pressed() const { return index != 0 && index > 0 ? index : 0; }
    } dimmer;
  };
  // Type of event.
  enum Type : uint8_t {
    BUTTON_CLICK = 0x00,
    BUTTON_DOUBLE_CLICK = 0x01,
    BUTTON_LONG_PRESS = 0x02,
    BUTTON_TRIPLE_CLICK = 0x03,
    KNOB = 0x03,
    DIMMER = 0x04,
    // SHORT_PRESS = 0x05,
    // LONG_PRESS = 0x06,
    _UNINITIALIZED = 0xFF,
  } type = _UNINITIALIZED;

  /**
   * Helper method for debugging purpose.
   * @param button_event button event object
   */
  std::string str() const;
  /**
   * Helper method for logging purpose.
   */
  void dump(const char *TAG) const;
} PACKED;

// Object returned by MIID_WATER_BOIL.
struct WaterBoil {
  uint8_t power;
  uint8_t temperature;

  // power, 0 - off, 1 - on
  bool get_power() const { return power; }
  // temperature in °C with 0 accuracy decimals
  float get_temperature() const { return temperature; };
} PACKED;

// Object returned by MIID_XIAOBEI_TOOTHBRUSH_EVENT.
struct ToothbrushEvent {
  enum Type : uint8_t { BRUSHING_START = 0, BRUSHING_END = 1 };
  // Types of:
  //  0: Start of brushing
  //  1: End of brushing
  Type type;
  // Timestamp: UTC time (time_t)
  uint32_t timestamp;
  // Score (optional): This parameter can be added to the end of brushing event: the score of this brushing, 0~100
  uint8_t score;
} PACKED;

struct BLEObject {
  BLEObject() : id(MIID_UNKNOWN) {}
  BLEObject(const uint8_t *encrypted_begin, const uint8_t *encrypted_end)
      : id(MIID_UNKNOWN), data(encrypted_begin, encrypted_end) {}
  BLEObject(RawBLEObject *obj) : id(obj->id), data(obj->data, obj->data + obj->data_len) {}

  MIID id;
  // sensor data or whole encrypted data when id==MIID_UNKNOWN
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

  // get 24 bit unsigned value.
  optional<uint32_t> get_uint24() const {
    if (data.size() == sizeof(uint32_t) - 1) {
      return (*reinterpret_cast<const uint32_t *>(data.data())) & 0x00FFFFFF;
    }
    return {};
  }

  // get 32 or 24 bit unsigned value.
  optional<uint32_t> get_uint32() const {
    if (data.size() == sizeof(uint32_t)) {
      return *reinterpret_cast<const uint32_t *>(data.data());
    }
    return {};
  }

  // get typed struct value. example: obj.get_typed<MyData>();
  template<typename T> const T *get_typed() const {
    if (data.size() == sizeof(T)) {
      return reinterpret_cast<const T *>(data.data());
    }
    return nullptr;
  }

  // get float value.
  optional<float> get_float() const {
    if (data.size() == sizeof(float)) {
      return *reinterpret_cast<const float *>(data.data());
    }
    return {};
  }

  /**
   * Value for MIID_BATTERY.
   * @return battery level in %
   */
  optional<uint8_t> get_battery_level() const;
  /**
   * Value for MIID_DOOR_SENSOR.
   * @return See MIID_DOOR_SENSOR for detailed description for returning value
   */
  optional<uint8_t> get_door_sensor() const;
  /**
   * Value for MIID_IDLE_TIME.
   * @return idle time in seconds
   */
  optional<uint32_t> get_idle_time() const;
  /**
   * Value for MIID_TIMEOUT.
   * @return timeout in seconds
   */
  optional<uint32_t> get_timeout() const;
  /**
   * Value for MIID_MOTION_WITH_LIGHT_EVENT.
   * @return illuminance in lux
   */
  optional<uint32_t> get_motion_with_light_event() const;
  /**
   * Value for MIID_FLOODING.
   * @return true on flooding or false if not.
   */
  optional<bool> get_flooding() const;
  /**
   * Value for MIID_LIGHT_INTENSITY.
   * @return true for a strong and false for a weak light
   */
  optional<bool> get_light_intensity() const;
  /**
   * Value for MIID_TEMPERATURE.
   * @return temperature in °C
   */
  optional<float> get_temperature() const;
  /**
   * Value for MIID_HUMIDITY.
   * @return humidity in %
   */
  optional<float> get_humidity() const;
  /**
   * Value for MIID_TEMPERATURE_HUMIDITY.
   * @return TemperatureHumidity object
   */
  const TemperatureHumidity *get_temperature_humidity() const;
  /**
   * Value for MIID_BUTTON_EVENT.
   * @return ButtonEvent object
   */
  const ButtonEvent *get_button_event() const;

  /**
   * Value for MIID_ILLUMINANCE
   * @return illuminance in lux, range 0-120000
   */
  optional<float> get_illuminance() const;

  /**
   * Value for MIID_PAIRING_EVENT.
   * @return Object ID to be paired, such as key events (0x1001)
   */
  optional<MIID> get_pairing_object() const;

  /**
   * Value for MIID_WATER_BOIL.
   * @return WaterBoil object
   */
  const WaterBoil *get_water_boil() const;

  /**
   * Value for MIID_MIAOMIAOCE_BATTERY.
   * @return battery level in %
   */
  optional<uint8_t> get_miaomiaoce_battery_level_1003() const;

  /**
   * Value for MIID_MIAOMIAOCE_TEMPERATURE.
   * @return temperature.
   */
  optional<float> get_miaomiaoce_temperature_1001() const;

  /**
   * Value for MIID_MIAOMIAOCE_HUMIDITY.
   * @return humidity percentage, %
   */
  optional<float> get_miaomiaoce_humidity_1008() const;

  /**
   * Value for MIID_MIAOMIAOCE_HUMIDITY_1002.
   * @return humidity percentage, %
   */
  optional<float> get_miaomiaoce_humidity_1002() const;

  /**
   * Value for MIID_CONSUMABLE.
   * @return consumable percentage, %
   */
  optional<uint8_t> get_consumable() const;

  /**
   * Value for MIID_XIAOBEI_TOOTHBRUSH_EVENT
   * @return toothbrush_event
   */
  optional<ToothbrushEvent> get_toothbrush_event() const;
};

}  // namespace miot
}  // namespace esphome
