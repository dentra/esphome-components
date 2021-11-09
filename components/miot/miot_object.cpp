#ifdef ARDUINO_ARCH_ESP32
#include "inttypes.h"
#include "esphome/core/log.h"
#include "miot_object.h"

#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_DEBUG
#define CHECK_MIID(miid) \
  if (this->id != miid) { \
    ESP_LOGW(TAG, "BLEObject.id = %04X does not match " #miid " [%04X]", this->id, miid); \
    return {}; \
  }
#else
#define CHECK_MIID(miid)
#endif

namespace esphome {
namespace miot {

static const char *const TAG = "miot.object";

optional<uint8_t> BLEObject::get_battery_level() const {
  CHECK_MIID(MIID_BATTERY);
  auto battery_level = this->get_uint8();
  if (battery_level.has_value()) {
    ESP_LOGD(TAG, "Battery level: %" PRIu8 " %%", *battery_level);
  }
  return battery_level;
}

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
      ESP_LOGD(TAG, "Button click: %" PRIu8 ", value: %" PRIi8, res.index, res.value);
      break;
    case ButtonEvent::DOUBLE_CLICK:
      ESP_LOGD(TAG, "Button double click: %" PRIu8 ", value: %" PRIi8, res.index, res.value);
      break;
    case ButtonEvent::TRIPLE_CLICK_OR_ROTATE_KNOB: {
      if (res.value == 0) {
        ESP_LOGD(TAG, "Button triple click: %" PRIu8 ", value: %" PRIi8, res.index, res.value);
      } else if (res.index == 0) {
        ESP_LOGD(TAG, "Button short press knob, value: %" PRIi8, res.value);
      } else if (res.index == 1) {
        ESP_LOGD(TAG, "Button long press knob, value: %" PRIi8, res.value);
      } else {
        ESP_LOGD(TAG, "Button press knob: %" PRIu8 ", value: %" PRIi8, res.index, res.value);
      }
      break;
    }
    case ButtonEvent::LONG_PRESS:
      ESP_LOGD(TAG, "Button long press: %" PRIu8 ", value: %" PRIi8, res.index, res.value);
      break;
    case ButtonEvent::ROTATE: {
      if (res.index == 0) {
        ESP_LOGD(TAG, "Button rotate %s knob, value: %" PRIi8, res.value < 0 ? "left" : "right", res.value);
      } else {
        int8_t dimmer = res.index;
        ESP_LOGD(TAG, "Button rotate %s (pressed) knob: %" PRIi8 ", value: %" PRIi8, dimmer < 0 ? "left" : "right",
                 dimmer, res.value);
      }
      break;
    }
    default:
      ESP_LOGD(TAG, "Button unknown event %02" PRIx8 ": %" PRIu8 ", value: %" PRIi8, res.type, res.index, res.value);
      break;
  }
  return res;
}

optional<float> BLEObject::get_illuminance() const {
  CHECK_MIID(MIID_ILLUMINANCE);
  const auto illuminance = this->get_uint24();
  if (illuminance.has_value()) {
    ESP_LOGD(TAG, "Illuminance %u lx", *illuminance);
  }
  return *illuminance;
}

optional<MIID> BLEObject::get_pairing_object() const {
  CHECK_MIID(MIID_PAIRING_EVENT);
  const auto event = this->get_uint16();
  if (event.has_value()) {
    ESP_LOGD(TAG, "Paring object %04X", *event);
  }
  return static_cast<MIID>(*event);
}

optional<WaterBoil> BLEObject::get_water_boil() const {
  CHECK_MIID(MIID_WATER_BOIL);
  struct _WaterBoil {
    uint8_t power;
    uint8_t temperature;
  };
  const auto typed = this->get_typed<_WaterBoil>();
  if (!typed.has_value()) {
    return {};
  }
  WaterBoil res;
  res.power = (*typed)->power != 0;
  ESP_LOGD(TAG, "Water Boil Power %s", ONOFF(res.power));
  res.temperature = (*typed)->temperature;
  ESP_LOGD(TAG, "Water Boil Temperature %.1f °C", res.temperature);
  return res;
}

optional<uint8_t> BLEObject::get_miaomiaoce_battery_level() const {
  CHECK_MIID(MIID_MIAOMIAOCE_BATTERY);
  auto battery_level = this->get_uint8();
  if (battery_level.has_value()) {
    ESP_LOGD(TAG, "Battery level: %" PRIu8 " %%", *battery_level);
  }
  return battery_level;
}

optional<float> BLEObject::get_miaomiaoce_temperature() const {
  CHECK_MIID(MIID_MIAOMIAOCE_TEMPERATURE);
  const auto temperature = this->get_float();
  if (!temperature.has_value()) {
    return temperature;
  }
  const float res = *temperature;
  ESP_LOGD(TAG, "Temperature %.1f °C", res);
  return res;
}

optional<float> BLEObject::get_miaomiaoce_humidity() const {
  CHECK_MIID(MIID_MIAOMIAOCE_HUMIDITY);
  const auto humidity = this->get_float();
  if (!humidity.has_value()) {
    return humidity;
  }
  const float res = *humidity;
  ESP_LOGD(TAG, "Humidity %.1f %%", res);
  return res;
}

}  // namespace miot
}  // namespace esphome
#endif
