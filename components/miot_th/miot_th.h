#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_th {

class MiotTH : public miot::MiotComponent, public sensor::Sensor {
 public:
  void dump_config() override;

  void set_humidity(sensor::Sensor *humidity) { this->humidity_ = humidity; }

 protected:
  sensor::Sensor *humidity_{nullptr};

  bool process_object_(const miot::BLEObject &obj) override;

  void process_temperature_(const miot::BLEObject &obj);
  void process_humidity_(const miot::BLEObject &obj);
  void process_temperature_humidity_(const miot::BLEObject &obj);
  void process_miaomiaoce_temperature_1001_(const miot::BLEObject &obj);
  void process_miaomiaoce_humidity_1008_(const miot::BLEObject &obj);
  void process_miaomiaoce_humidity_1002_(const miot::BLEObject &obj);
};

}  // namespace miot_th
}  // namespace esphome
