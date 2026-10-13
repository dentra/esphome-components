#pragma once

#include <map>
#include "esphome/core/component.h"
#include "esphome/core/version.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/switch/switch.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_explorer {

class MiotExplorerTextSensor : public text_sensor::TextSensor {
 public:
  explicit MiotExplorerTextSensor(std::string &&name) : name_(std::move(name)) {
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2026, 3, 0)
    this->configure_entity_(this->name_.c_str(), 0, 0);
#else
    this->set_name(this->name_.c_str());
#endif
  }

 protected:
  std::string name_;
};

class MiotExplorer : public miot::MiotComponent, public text_sensor::TextSensor {
 public:
  void dump_config() override;

  bool process_mibeacon(const miot::MiBeacon &mib) override;

  void set_consumable(sensor::Sensor *consumable) { this->consumable_ = consumable; }
  void set_max_sensors(uint8_t max_sensors) { this->max_sensors_ = max_sensors; }

 protected:
  std::map<miot::MIID, MiotExplorerTextSensor *> sensors_{};
  uint8_t max_sensors_{};
  sensor::Sensor *consumable_{};

  bool process_object_(const miot::BLEObject &obj) override;
  void process_string_(miot::MIID miid, const std::string &name, const std::string &data);
  void process_uint8_(miot::MIID miid, const std::string &name, const optional<uint8_t> &value);
  void process_uint16_(miot::MIID miid, const std::string &name, const optional<uint16_t> &value);
  void process_uint32_(miot::MIID miid, const std::string &name, const optional<uint32_t> &value);
  void process_pairing_event_(miot::MIID miid, const std::string &name, const optional<miot::MIID> &value);
  void process_bool_(miot::MIID miid, const std::string &name, const optional<bool> &value);
  void process_float_(miot::MIID miid, const std::string &name, const optional<float> &value);
  void process_temperature_humidity_(miot::MIID miid, const std::string &name, const miot::TemperatureHumidity *th);
  void process_button_event_(miot::MIID miid, const std::string &name, const miot::ButtonEvent *button_event);
  void process_water_boil_(miot::MIID miid, const std::string &name, const miot::WaterBoil *water_boil);
};

}  // namespace miot_explorer
}  // namespace esphome
