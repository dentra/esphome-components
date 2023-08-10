#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_cgpr1 {

class MiotCGPR1 : public miot::MiotComponent, public binary_sensor::BinarySensor {
 public:
  MiotCGPR1() { this->product_id_ = 0x0A83; }

  void dump_config() override;

  void set_light(binary_sensor::BinarySensor *light) { this->light_ = light; }
  void set_idle_time(sensor::Sensor *idle_time) { this->idle_time_ = idle_time; }
  void set_illuminance(sensor::Sensor *illuminance) { this->illuminance_ = illuminance; }
  void set_detect_no_motion_asap(bool detect_no_motion_asap) { this->detect_no_motion_asap_ = detect_no_motion_asap; }

 protected:
  binary_sensor::BinarySensor *light_{};
  sensor::Sensor *idle_time_{};
  sensor::Sensor *illuminance_{};
  bool detect_no_motion_asap_{};

  bool process_object_(const miot::BLEObject &obj) override;

  void process_idle_time_(const miot::BLEObject &obj);
  void process_motion_with_light_event_(const miot::BLEObject &obj);
  void process_illuminance_(const miot::BLEObject &obj);
  void process_light_intensity_(const miot::BLEObject &obj);
};

}  // namespace miot_cgpr1
}  // namespace esphome
