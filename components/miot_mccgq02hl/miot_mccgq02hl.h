#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "../miot/miot.h"

#ifdef ARDUINO_ARCH_ESP32

namespace esphome {
namespace miot_mccgq02hl {

class MiotMCCGQ02HL : public miot::MiotComponent, public binary_sensor::BinarySensorInitiallyOff {
 public:
  void dump_config() override;

  uint16_t get_product_id() const override { return 0x098b; }
  const char *get_product_code() const override { return "MCCGQ02HL"; }

  void set_light(binary_sensor::BinarySensor *light) { this->light_ = light; }
  void set_alert(binary_sensor::BinarySensor *alert) { this->alert_ = alert; }

 protected:
  binary_sensor::BinarySensor *light_{nullptr};
  binary_sensor::BinarySensor *alert_{nullptr};

  void process_object_(const miot::BLEObject &obj) override;

  void process_door_sensor_(const miot::BLEObject &obj);
  void process_light_intensity_(const miot::BLEObject &obj);
};

}  // namespace miot_mccgq02hl
}  // namespace esphome

#endif
