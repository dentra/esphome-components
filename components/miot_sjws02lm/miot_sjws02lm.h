#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../miot/miot.h"

namespace esphome::miot_sjws02lm {

class MiotSJWS02LM : public miot::MiotComponent, public binary_sensor::BinarySensor {
 public:
  MiotSJWS02LM() { this->product_id_ = 0x6375; }

  void dump_config() override;

 protected:
  bool process_object_(const miot::BLEObject &obj) override;

  void process_flooding_(const miot::BLEObject &obj);
};

}  // namespace esphome::miot_sjws02lm
