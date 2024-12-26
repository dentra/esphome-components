#include "inttypes.h"
#include "esphome/core/log.h"
#include "miot_object.h"

#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_DEBUG
#define CHECK_MIID(miid) \
  if (this->id != miid) { \
    ESP_LOGW(TAG, "BLEObject.id %04" PRIX16 " does not match %04" PRIX16 " (" #miid ")", this->id, miid); \
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
    ESP_LOGD(TAG, "Idle time: %" PRIu32 " s", *idle_time);
  }
  return idle_time;
}

optional<uint32_t> BLEObject::get_timeout() const {
  CHECK_MIID(MIID_TIMEOUT);
  const auto timeout = this->get_uint32();
  if (timeout.has_value()) {
    ESP_LOGD(TAG, "Timeout: %" PRIu32 " s", *timeout);
  }
  return timeout;
}

optional<uint32_t> BLEObject::get_motion_with_light_event() const {
  CHECK_MIID(MIID_MOTION_WITH_LIGHT_EVENT);
  const auto illuminance = this->get_uint24();
  if (illuminance.has_value()) {
    ESP_LOGD(TAG, "Motion with light: %" PRIu32 " lx", *illuminance);
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
    return {};
  }
  const float res = *temperature * 0.1f;
  ESP_LOGD(TAG, "Temperature %.1f °C", res);
  return res;
}

optional<float> BLEObject::get_humidity() const {
  CHECK_MIID(MIID_HUMIDITY);
  const auto humidity = this->get_uint16();
  if (!humidity.has_value()) {
    return {};
  }
  const float res = *humidity * 0.1f;
  ESP_LOGD(TAG, "Humidity %.1f %%", res);
  return res;
}

const TemperatureHumidity *BLEObject::get_temperature_humidity() const {
  CHECK_MIID(MIID_TEMPERATURE_HUMIDITY);
  const auto th = this->get_typed<TemperatureHumidity>();
  if (th != nullptr) {
    ESP_LOGD(TAG, "Temperature %.1f °C", th->get_temperature());
    ESP_LOGD(TAG, "Humidity %.1f %%", th->get_humidity());
  }
  return th;
}

std::string ButtonEvent::str() const {
  switch (this->type) {
    case ButtonEvent::BUTTON_CLICK:
      return str_sprintf("Button click: %" PRIu16, this->button.index);
    case ButtonEvent::BUTTON_DOUBLE_CLICK:
      return str_sprintf("Button double click: %" PRIu16, this->button.index);

    case ButtonEvent::BUTTON_LONG_PRESS:
      return str_sprintf("Button long press: %" PRIu16, this->button.index);

    case ButtonEvent::KNOB: {
      uint8_t value = this->knob.short_press();
      if (value != 0) {
        return str_sprintf("Rotating knob short press, value: %" PRIu8, value);
      }
      value = this->knob.long_press();
      if (value != 0) {
        return str_sprintf("Rotating knob long press, value: %" PRIu8, value);
      }
      return str_sprintf("Rotating knob unknown event: index=%" PRIu8 ", value:=%" PRIu8, this->knob.index,
                         this->knob.value);
    }
    case ButtonEvent::DIMMER: {
      uint8_t value = this->dimmer.left();
      if (value != 0) {
        return str_sprintf("Dimmer rotate left value: %" PRIu8, value);
      }
      value = this->dimmer.right();
      if (value != 0) {
        return str_sprintf("Dimmer rotate right value: %" PRIu8, value);
      }
      value = this->dimmer.left_pressed();
      if (value != 0) {
        return str_sprintf("Dimmer rotate left (pressed) value: %" PRIu8, value);
      }
      value = this->dimmer.right_pressed();
      if (value != 0) {
        return str_sprintf("Dimmer rotate right (pressed) value: %" PRIu8, value);
      }
      return str_sprintf("Dimmer unknown event: index=%" PRIi8 ", value:=%" PRIi8, this->dimmer.index,
                         this->dimmer.value);
    }
    default:
      return str_sprintf("Button unknown event %02" PRIx8 ": %04" PRIX16, this->type, this->button.index);
  }
}

void ButtonEvent::dump(const char *TAG) const { ESP_LOGD(TAG, this->str().c_str()); }

const ButtonEvent *BLEObject::get_button_event() const {
  CHECK_MIID(MIID_BUTTON_EVENT);
  const auto button_event = this->get_typed<ButtonEvent>();
  if (button_event != nullptr) {
    button_event->dump(TAG);
  }
  return button_event;
}

optional<float> BLEObject::get_illuminance() const {
  CHECK_MIID(MIID_ILLUMINANCE);
  const auto illuminance = this->get_uint24();
  if (illuminance.has_value()) {
    ESP_LOGD(TAG, "Illuminance %" PRIu32 " lx", *illuminance);
  }
  return *illuminance;
}

optional<MIID> BLEObject::get_pairing_object() const {
  CHECK_MIID(MIID_PAIRING_EVENT);
  const auto event = this->get_uint16();
  if (event.has_value()) {
    ESP_LOGD(TAG, "Paring object %04" PRIX16, *event);
  }
  return static_cast<MIID>(*event);
}

const WaterBoil *BLEObject::get_water_boil() const {
  CHECK_MIID(MIID_WATER_BOIL);
  const auto water_boil = this->get_typed<WaterBoil>();
  if (water_boil != nullptr) {
    ESP_LOGD(TAG, "Water Boil Power %s", ONOFF(water_boil->get_power()));
    ESP_LOGD(TAG, "Water Boil Temperature %.1f °C", water_boil->get_temperature());
  }
  return water_boil;
}

optional<uint8_t> BLEObject::get_miaomiaoce_battery_level_1003() const {
  CHECK_MIID(MIID_MIAOMIAOCE_BATTERY_1003);
  auto battery_level = this->get_uint8();
  if (battery_level.has_value()) {
    ESP_LOGD(TAG, "Battery level: %" PRIu8 " %%", *battery_level);
  }
  return battery_level;
}

optional<float> BLEObject::get_miaomiaoce_temperature_1001() const {
  CHECK_MIID(MIID_MIAOMIAOCE_TEMPERATURE_1001);
  const auto temperature = this->get_float();
  if (!temperature.has_value()) {
    return {};
  }
  const float res = *temperature;
  ESP_LOGD(TAG, "Temperature %.1f °C", res);
  return res;
}

optional<float> BLEObject::get_miaomiaoce_humidity_1008() const {
  CHECK_MIID(MIID_MIAOMIAOCE_HUMIDITY_1008);
  const auto humidity = this->get_float();
  if (!humidity.has_value()) {
    return {};
  }
  const float res = *humidity;
  ESP_LOGD(TAG, "Humidity %.1f %%", res);
  return res;
}

optional<float> BLEObject::get_miaomiaoce_humidity_1002() const {
  CHECK_MIID(MIID_MIAOMIAOCE_HUMIDITY_1002);
  const auto humidity = this->get_uint8();
  if (!humidity.has_value()) {
    return {};
  }
  const float res = *humidity;
  ESP_LOGD(TAG, "Humidity %.1f %%", res);
  return res;
}

optional<uint8_t> BLEObject::get_consumable() const {
  CHECK_MIID(MIID_CONSUMABLE);
  auto consumable = this->get_uint8();
  if (consumable.has_value()) {
    ESP_LOGD(TAG, "Consumable: %" PRIu8 " %%", *consumable);
  }
  return consumable;
}

optional<ToothbrushEvent> BLEObject::get_toothbrush_event() const {
  CHECK_MIID(MIID_XIAOBEI_TOOTHBRUSH_EVENT);

  struct ToothbrushEventStart {
    ToothbrushEvent::Type type;
    uint32_t timestamp;
  } PACKED;

  ToothbrushEvent toothbrush_event{};
  if (this->data.size() == sizeof(ToothbrushEventStart)) {
    auto toothbrush_start_event = this->get_typed<ToothbrushEventStart>();
    toothbrush_event.type = toothbrush_start_event->type;
    toothbrush_event.timestamp = toothbrush_start_event->timestamp;
  } else if (this->data.size() == sizeof(ToothbrushEvent)) {
    auto toothbrush_end_event = this->get_typed<ToothbrushEvent>();
    if (toothbrush_end_event) {
      toothbrush_event.type = toothbrush_end_event->type;
      toothbrush_event.timestamp = toothbrush_end_event->timestamp;
      toothbrush_event.score = toothbrush_end_event->score;
    }
  } else {
    ESP_LOGD(TAG, "Toothbrush unknown event: %s", format_hex_pretty(this->data.data(), this->data.size()).c_str());
    return {};
  }

  if (toothbrush_event.type == ToothbrushEvent::BRUSHING_START) {
    ESP_LOGD(TAG, "Toothbrush start event: timestamp=%" PRIu32, toothbrush_event.timestamp);
  } else {
    ESP_LOGD(TAG, "Toothbrush end event: timestamp=%" PRIu32 ", score=%" PRIu8, toothbrush_event.timestamp, toothbrush_event.score);
  }

  return toothbrush_event;
}

}  // namespace miot
}  // namespace esphome
