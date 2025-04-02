#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_motion {

class MiotMotion : public miot::MiotComponent, public binary_sensor::BinarySensor {
  SUB_BINARY_SENSOR(light);
  SUB_SENSOR(idle_time);
  SUB_SENSOR(illuminance);

 public:
  void dump_config() override;
  void setup() override;

  void set_reset_timeout(uint32_t reset_timeout) { this->reset_timeout_ = reset_timeout; }

 protected:
  uint32_t reset_timeout_{};

  bool process_object_(const miot::BLEObject &obj) override;

  void process_idle_time_(uint32_t idle_time, bool has_time);
  void process_idle_time_(const miot::BLEObject &obj);
  void process_motion_with_light_event_(const miot::BLEObject &obj);
  void process_timeout_(const miot::BLEObject &obj);
  void process_light_intensity_(const miot::BLEObject &obj);
  void process_illuminance_(const miot::BLEObject &obj);
  void process_motion_with_illuminance_(float illuminance);
  void process_motion_with_illuminance_(const miot::BLEObject &obj);
  void process_motion_event_(const miot::BLEObject &obj);
  void process_no_motion_time_(const miot::BLEObject &obj);
};

}  // namespace miot_motion
}  // namespace esphome
