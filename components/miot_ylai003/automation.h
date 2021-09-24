#pragma once
#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/automation.h"
#include "../miot/miot.h"
#include "miot_ylai003.h"

namespace esphome {
namespace miot_ylai003 {

class MiotYLAI003Trigger : public Trigger<>, public miot::MiotListener {
 public:
  explicit MiotYLAI003Trigger(miot::ButtonEvent::Type type) : type_(type) {}

  uint16_t get_product_id() const override { return PRODUCT_ID; }

 protected:
  miot::ButtonEvent::Type type_;
  bool process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_ylai003
}  // namespace esphome
#endif
