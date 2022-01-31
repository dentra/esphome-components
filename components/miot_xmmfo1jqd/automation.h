#pragma once

#include "esphome/core/automation.h"
#include "../miot/miot.h"
#include "miot_xmmfo1jqd.h"

namespace esphome {
namespace miot_xmmfo1jqd {

class MiotXMMFO1JQDTrigger : public Trigger<>, public miot::MiotListener {
 public:
  explicit MiotXMMFO1JQDTrigger(miot::ButtonEvent::Type type, uint16_t index) : type_(type), index_(index) {}

  uint16_t get_product_id() const override { return PRODUCT_ID_XMMFO1JQD; }

 protected:
  miot::ButtonEvent::Type type_;
  uint16_t index_;
  bool process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_xmmfo1jqd
}  // namespace esphome
