#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "../miot/miot.h"

#ifdef ARDUINO_ARCH_ESP32

namespace esphome {
namespace miot_sjws01lm {

class MiotSJWS01LM : public miot::MiotComponent, public binary_sensor::BinarySensor {
 public:
  uint16_t get_product_id() const override { return 0x0863; }
  const char *get_product_code() const override { return "SJWS01LM"; }

  void dump_config() override;

 protected:
  void process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_sjws01lm
}  // namespace esphome

#endif
