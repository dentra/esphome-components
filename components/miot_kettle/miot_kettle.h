#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_kettle {

class MiotKettle : public miot::MiotComponent, public binary_sensor::BinarySensor {
 public:
  // Known ID: 0x045C - V-SK152
  uint16_t get_product_id() const override { return this->product_id_; }
  const char *get_product_code() const override { return "universal kettle"; }

  void dump_config() override;

  void set_temperature(sensor::Sensor *temperature) { this->temperature_ = temperature; }

  bool process_mibeacon(const miot::MiBeacon &mib);

 protected:
  uint16_t product_id_ = {};
  sensor::Sensor *temperature_{nullptr};

  bool process_object_(const miot::BLEObject &obj) override;

  void process_water_boil_(const miot::BLEObject &obj);
};

}  // namespace miot_kettle
}  // namespace esphome
