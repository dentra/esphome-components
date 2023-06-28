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
  LOG_SENSOR("  ", "Consumable", this->consumable_);
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
    case miot::MIID_MIAOMIAOCE_BATTERY_1003:
      this->process_default_(obj);
      break;
    case miot::MIID_CONSUMABLE: {
      auto consumable = obj.get_consumable();
      if (consumable.has_value()) {
        if (this->consumable_) {
          this->consumable_->publish_state(*obj.get_consumable());
        } else {
          this->process_string_(obj.id, "Consumable", to_string(*obj.get_consumable()));
        }
      }
      break;
    }
    case miot::MIID_PAIRING_EVENT:
      this->process_pairing_event_(obj.id, "Pairing object", obj.get_pairing_object());
      break;
    case miot::MIID_DOOR_SENSOR:
      this->process_uint8_(obj.id, "Opening", obj.get_door_sensor());
      break;
    case miot::MIID_IDLE_TIME:
      this->process_uint32_(obj.id, "Idle time", obj.get_idle_time());
      break;
    case miot::MIID_TIMEOUT:
      this->process_uint32_(obj.id, "Timeout", obj.get_timeout());
      break;
    case miot::MIID_MOTION_WITH_LIGHT_EVENT:
      this->process_uint32_(obj.id, "Motion with light", obj.get_motion_with_light_event());
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
      this->process_string_(obj.id, "", format_hex_pretty(obj.data));
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
    auto full = str_sprintf("%s [%04X] %s", this->get_name().c_str(), miid, name.c_str());
    sens->set_name(strdup(full.c_str()));
    App.register_text_sensor(sens);
    this->sensors_[miid] = sens;
  }
  sens->publish_state(data);
}

void MiotExplorer::process_uint8_(miot::MIID miid, const std::string &name, const optional<uint8_t> &value) {
  if (value.has_value()) {
    this->process_string_(miid, name, to_string(*value));
  }
}

void MiotExplorer::process_uint16_(miot::MIID miid, const std::string &name, const optional<uint16_t> &value) {
  if (value.has_value()) {
    this->process_string_(miid, name, to_string(*value));
  }
}

void MiotExplorer::process_pairing_event_(miot::MIID miid, const std::string &name, const optional<miot::MIID> &value) {
  if (value.has_value()) {
    this->process_string_(miid, name, to_string(*value));
  }
}

void MiotExplorer::process_uint32_(miot::MIID miid, const std::string &name, const optional<uint32_t> &value) {
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
                                                 const miot::TemperatureHumidity *th) {
  if (th != nullptr) {
    char tmp[16] = {};
    sprintf(tmp, "%.1f / %.1f", th->get_temperature(), th->get_humidity());
    this->process_string_(miid, name, std::string(tmp));
  }
}

void MiotExplorer::process_button_event_(miot::MIID miid, const std::string &name,
                                         const miot::ButtonEvent *button_event) {
  if (button_event != nullptr) {
    char tmp[64] = {};
    miot::ButtonEvent::str(tmp, *button_event);
    this->process_string_(miid, name, std::string(tmp));
  }
}

void MiotExplorer::process_water_boil_(miot::MIID miid, const std::string &name, const miot::WaterBoil *water_boil) {
  if (water_boil != nullptr) {
    char tmp[16] = {};
    sprintf(tmp, "%s / %.1f", ONOFF(water_boil->get_power()), water_boil->get_temperature());
    this->process_string_(miid, name, std::string(tmp));
  }
}

}  // namespace miot_explorer
}  // namespace esphome
