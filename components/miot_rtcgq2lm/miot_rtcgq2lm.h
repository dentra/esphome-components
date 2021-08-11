#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "../miot/miot.h"

#ifdef ARDUINO_ARCH_ESP32

namespace esphome {
namespace miot_rtcgq2lm {

class MiotRTCGQ2LM : public miot::MiotComponent, public binary_sensor::BinarySensor {
 public:
  uint16_t get_product_id() const override { return 0x0A8D; }
  const char *get_product_code() const override { return "RTCGQ2LM"; }

  void dump_config() override;

  void set_light(binary_sensor::BinarySensor *light) { this->light_ = light; }
  void set_idle_time(sensor::Sensor *idle_time) { this->idle_time_ = idle_time; }
  void set_illuminance(sensor::Sensor *illuminance) { this->illuminance_ = illuminance; }

 protected:
  binary_sensor::BinarySensor *light_{nullptr};
  sensor::Sensor *idle_time_{nullptr};
  sensor::Sensor *illuminance_{nullptr};

  void process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_rtcgq2lm
}  // namespace esphome

#endif
