#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_sjws02lm {

class MiotSJWS02LM : public miot::MiotComponent, public binary_sensor::BinarySensor {
 public:
  MiotSJWS02LM() { this->product_id_ = 0x6375; }

  void dump_config() override;

  void set_bottom(binary_sensor::BinarySensor *bottom) { this->bottom_ = bottom; }
  void set_top(binary_sensor::BinarySensor *top) { this->top_ = top; }

 protected:
  binary_sensor::BinarySensor *bottom_{};
  binary_sensor::BinarySensor *top_{};
  optional<bool> bottom_state_{};
  optional<bool> top_state_{};

  bool process_object_(const miot::BLEObject &obj) override;

  void process_water_leak_event_(const miot::BLEObject &obj);
  void publish_bottom_(bool state);
  void publish_top_(bool state);
  void publish_water_leak_();
};

}  // namespace miot_sjws02lm
}  // namespace esphome
