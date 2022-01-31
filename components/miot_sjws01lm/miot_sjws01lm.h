#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_sjws01lm {

class MiotSJWS01LM : public miot::MiotComponent, public binary_sensor::BinarySensor {
 public:
  uint16_t get_product_id() const override { return 0x0863; }
  const char *get_product_code() const override { return "SJWS01LM"; }

  void dump_config() override;

 protected:
  bool process_object_(const miot::BLEObject &obj) override;

  void process_flooding_(const miot::BLEObject &obj);
};

}  // namespace miot_sjws01lm
}  // namespace esphome
