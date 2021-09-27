#pragma once
#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_mjyd02yla {

class MiotMJYD02YLA : public miot::MiotComponent, public binary_sensor::BinarySensor {
 public:
  uint16_t get_product_id() const override { return 0x07F6; }
  const char *get_product_code() const override { return "MJYD02YL-A"; }

  void dump_config() override;

  void set_light(binary_sensor::BinarySensor *light) { this->light_ = light; }
  void set_idle_time(sensor::Sensor *idle_time) { this->idle_time_ = idle_time; }

 protected:
  binary_sensor::BinarySensor *light_{nullptr};
  sensor::Sensor *idle_time_{nullptr};

  bool process_object_(const miot::BLEObject &obj) override;

  void process_idle_time_(const miot::BLEObject &obj);
  void process_motion_with_light_event_(const miot::BLEObject &obj);
  void process_illuminance_(const miot::BLEObject &obj);
};

}  // namespace miot_mjyd02yla
}  // namespace esphome

#endif
