#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_mccgq02hl {

class MiotMCCGQ02HL : public miot::MiotComponent, public binary_sensor::BinarySensorInitiallyOff {
 public:
  MiotMCCGQ02HL() { this->product_id_ = 0x098b; }
  void dump_config() override;

  void set_light(binary_sensor::BinarySensor *light) { this->light_ = light; }
  void set_alert(binary_sensor::BinarySensor *alert) { this->alert_ = alert; }

 protected:
  binary_sensor::BinarySensor *light_{};
  binary_sensor::BinarySensor *alert_{};

  bool process_object_(const miot::BLEObject &obj) override;

  void process_door_sensor_(const miot::BLEObject &obj);
  void process_light_intensity_(const miot::BLEObject &obj);
};

}  // namespace miot_mccgq02hl
}  // namespace esphome
