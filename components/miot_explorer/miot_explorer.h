#pragma once
#ifdef ARDUINO_ARCH_ESP32
#include <map>
#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/switch/switch.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_explorer {

class MiotExplorer : public miot::MiotComponent, public text_sensor::TextSensor {
 public:
  void dump_config() override;

  uint16_t get_product_id() const override { return product_id_; }
  const char *get_product_code() const override { return "Explorer"; }

  bool process_mibeacon(const miot::MiBeacon &mib) override;

 protected:
  uint16_t product_id_;
  std::map<miot::MIID, EntityBase *> sensors_ = {};

  bool process_object_(const miot::BLEObject &obj) override;
  void process_any_(miot::MIID miid, const std::string &name, const std::string &data);
  void process_any_(miot::MIID miid, const std::string &name, const optional<uint8_t> &value);
  void process_any_(miot::MIID miid, const std::string &name, const optional<uint16_t> &value);
  void process_any_(miot::MIID miid, const std::string &name, const optional<miot::MIID> &value);
  void process_any_(miot::MIID miid, const std::string &name, const optional<uint32_t> &value);
  void process_any_(miot::MIID miid, const std::string &name, const optional<bool> &value);
  void process_any_(miot::MIID miid, const std::string &name, const optional<float> &value);
  void process_any_(miot::MIID miid, const std::string &name, const optional<const miot::TemperatureHumidity> &value);
  void process_any_(miot::MIID miid, const std::string &name, const optional<const miot::ButtonEvent> &value);
};

}  // namespace miot_explorer
}  // namespace esphome

#endif
