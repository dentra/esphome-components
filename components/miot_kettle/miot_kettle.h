#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_kettle {

// Known product ID: 0x045C - V-SK152
class MiotKettle : public miot::MiotComponent, public binary_sensor::BinarySensor {
 public:
  void dump_config() override;
  void set_temperature(sensor::Sensor *temperature) { this->temperature_ = temperature; }

 protected:
  sensor::Sensor *temperature_{nullptr};

  bool process_object_(const miot::BLEObject &obj) override;

  void process_water_boil_(const miot::BLEObject &obj);
};

}  // namespace miot_kettle
}  // namespace esphome
