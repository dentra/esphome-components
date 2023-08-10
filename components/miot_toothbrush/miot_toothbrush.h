#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_toothbrush {

class MiotToothbrush : public miot::MiotComponent, public binary_sensor::BinarySensor {
 public:
  void dump_config() override;

  void set_score(sensor::Sensor *score) { this->score_ = score; }
  void set_consumable_level(sensor::Sensor *consumable_level) { this->consumable_level_ = consumable_level; }
  void set_brushing_time(sensor::Sensor *brushing_time) { this->brushing_time_ = brushing_time; }

 protected:
  sensor::Sensor *score_{};
  sensor::Sensor *consumable_level_{};
  sensor::Sensor *brushing_time_{};
  time_t brushing_start_{};

  bool process_object_(const miot::BLEObject &obj) override;

  void process_toothbrush_event_(const miot::BLEObject &obj);
  void process_consumable_(const miot::BLEObject &obj);
};

}  // namespace miot_toothbrush
}  // namespace esphome
