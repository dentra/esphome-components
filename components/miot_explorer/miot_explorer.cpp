#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/log.h"
#include "esphome/core/application.h"

#include "miot_explorer.h"

namespace esphome {
namespace miot_explorer {

static const char *const TAG = "miot_explorer";

void MiotExplorer::dump_config() {
  this->dump_config_(TAG);
  ESP_LOGCONFIG(TAG, "Product ID: %04X", this->product_id_);
  LOG_TEXT_SENSOR("", "Explorer", this);
}

bool MiotExplorer::process_mibeacon(const miot::MiBeacon &mib) {
  if (this->product_id_ == 0) {
    this->product_id_ = mib.product_id;
    char tmp[6] = {};
    sprintf(tmp, "%04X", this->product_id_);
    this->publish_state(tmp);
  }
  return MiotListener::process_mibeacon(mib);
}

bool MiotExplorer::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_BATTERY:
      this->process_default_(obj);
      break;
    case miot::MIID_PAIRING_EVENT:
      this->process_pairing_event_(obj.id, "Pairing object", obj.get_pairing_object());
      break;
    case miot::MIID_DOOR_SENSOR:
      this->process_any_(obj.id, "Opening", obj.get_door_sensor());
      break;
    case miot::MIID_IDLE_TIME:
      this->process_any_(obj.id, "Idle time", obj.get_idle_time());
      break;
    case miot::MIID_TIMEOUT:
      this->process_any_(obj.id, "Timeout", obj.get_timeout());
      break;
    case miot::MIID_MOTION_WITH_LIGHT_EVENT:
      this->process_any_(obj.id, "Motion with light", obj.get_motion_with_light_event());
      break;
    case miot::MIID_FLOODING:
      this->process_bool_(obj.id, "Flooding", obj.get_flooding());
      break;
    case miot::MIID_LIGHT_INTENSITY:
      this->process_bool_(obj.id, "Light intensity", obj.get_light_intensity());
      break;
    case miot::MIID_TEMPERATURE:
      this->process_float_(obj.id, "Temperature", obj.get_temperature());
      break;
    case miot::MIID_HUMIDITY:
      this->process_float_(obj.id, "Humidity", obj.get_humidity());
      break;
    case miot::MIID_TEMPERATURE_HUMIDITY:
      this->process_temperature_humidity_(obj.id, "Temperture/Humidity", obj.get_temperature_humidity());
      break;
    case miot::MIID_BUTTON_EVENT:
      this->process_button_event_(obj.id, "Button event", obj.get_button_event());
      break;
    case miot::MIID_ILLUMINANCE:
      this->process_float_(obj.id, "Illuminance", obj.get_illuminance());
      break;
    case miot::MIID_WATER_BOIL:
      this->process_water_boil_(obj.id, "Water Boil", obj.get_water_boil());
      break;
    default:
      this->process_string_(obj.id, "", hexencode(obj.data));
      break;
  }
  return true;
}

void MiotExplorer::process_string_(miot::MIID miid, const std::string &name, const std::string &data) {
  auto it = this->sensors_.find(miid);
  TextSensor *sens;
  if (it != this->sensors_.end()) {
    sens = static_cast<TextSensor *>(it->second);
  } else {
    sens = new TextSensor();
    char tmp[16] = {};
    sprintf(tmp, " [%04X] ", miid);
    sens->set_name(this->get_name() + tmp + name);
    App.register_text_sensor(sens);
    this->sensors_[miid] = sens;
  }
  sens->publish_state(data);
}

void MiotExplorer::process_any_(miot::MIID miid, const std::string &name, const optional<uint8_t> &value) {
  if (value.has_value()) {
    this->process_string_(miid, name, to_string(*value));
  }
}

void MiotExplorer::process_any_(miot::MIID miid, const std::string &name, const optional<uint16_t> &value) {
  if (value.has_value()) {
    this->process_string_(miid, name, to_string(*value));
  }
}

void MiotExplorer::process_pairing_event_(miot::MIID miid, const std::string &name, const optional<miot::MIID> &value) {
  if (value.has_value()) {
    this->process_string_(miid, name, to_string(*value));
  }
}

void MiotExplorer::process_any_(miot::MIID miid, const std::string &name, const optional<uint32_t> &value) {
  if (value.has_value()) {
    this->process_string_(miid, name, to_string(*value));
  }
}

void MiotExplorer::process_bool_(miot::MIID miid, const std::string &name, const optional<bool> &value) {
  if (value.has_value()) {
    this->process_string_(miid, name, std::string(ONOFF(*value)));
  }
}

void MiotExplorer::process_float_(miot::MIID miid, const std::string &name, const optional<float> &value) {
  if (value.has_value()) {
    char tmp[16] = {};
    sprintf(tmp, "%.1f", *value);
    this->process_string_(miid, name, std::string(tmp));
  }
}

void MiotExplorer::process_temperature_humidity_(miot::MIID miid, const std::string &name,
                                                 const optional<const miot::TemperatureHumidity> &value) {
  if (value.has_value()) {
    char tmp[16] = {};
    sprintf(tmp, "%.1f / %.1f", (*value).temperature, (*value).humidity);
    this->process_string_(miid, name, std::string(tmp));
  }
}

void MiotExplorer::process_button_event_(miot::MIID miid, const std::string &name,
                                         const optional<const miot::ButtonEvent> &value) {
  if (value.has_value()) {
    char tmp[64] = {};
    const auto &res = *value;
    switch (res.type) {
      case miot::ButtonEvent::CLICK:
        sprintf(tmp, "click: %" PRIu8 ", value: %" PRIi8, res.index, res.value);
        break;
      case miot::ButtonEvent::DOUBLE_CLICK:
        sprintf(tmp, "double click: %" PRIu8 ", value: %" PRIi8, res.index, res.value);
        break;
      case miot::ButtonEvent::TRIPLE_CLICK_OR_ROTATE_KNOB: {
        if (res.value == 0) {
          sprintf(tmp, "Button triple click: %" PRIu8 ", value: %" PRIi8, res.index, res.value);
        } else if (res.index == 0) {
          sprintf(tmp, "Button short press knob, dimmer: %" PRIi8, res.value);
        } else if (res.index == 1) {
          sprintf(tmp, "Button long press knob, dimmer: %" PRIi8, res.value);
        } else {
          sprintf(tmp, "Button press knob: %" PRIu8 ", value: %" PRIi8, res.index, res.value);
        }
        break;
      }
      case miot::ButtonEvent::LONG_PRESS:
        sprintf(tmp, "long press: %" PRIu8 ", value: %" PRIi8, res.index, res.value);
        break;
      case miot::ButtonEvent::ROTATE: {
        if (res.index == 0) {
          sprintf(tmp, "Button rotate %s knob, dimmer: %" PRIi8, res.value < 0 ? "left" : "right", res.value);
        } else {
          int8_t dimmer = res.index;
          sprintf(tmp, "Button rotate %s (pressed) knob, dimmer: %" PRIi8 ", value: %" PRIi8,
                  dimmer < 0 ? "left" : "right", dimmer, res.value);
        }
        break;
      }
      default:
        sprintf(tmp, "Button unknown event %02" PRIx8 ": %" PRIu8 ", value: %" PRIi8, res.type, res.index, res.value);
        break;
    }
    this->process_string_(miid, name, std::string(tmp));
  }
}

void MiotExplorer::process_water_boil_(miot::MIID miid, const std::string &name,
                                       const optional<const miot::WaterBoil> &value) {
  if (value.has_value()) {
    char tmp[16] = {};
    sprintf(tmp, "%s / %.1f", ONOFF(value->power), value->temperature);
    this->process_string_(miid, name, std::string(tmp));
  }
}

}  // namespace miot_explorer
}  // namespace esphome

#endif
