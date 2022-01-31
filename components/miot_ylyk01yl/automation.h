#pragma once

#include "esphome/core/automation.h"
#include "../miot/miot.h"
#include "miot_ylyk01yl.h"

namespace esphome {
namespace miot_ylyk01yl {

class MiotYLYK01YLTrigger : public Trigger<>, public miot::MiotListener {
 public:
  explicit MiotYLYK01YLTrigger(miot::ButtonEvent::Type type, uint16_t index) : type_(type), index_(index) {}

  uint16_t get_product_id() const override { return PRODUCT_ID_YLYK01YL; }

 protected:
  miot::ButtonEvent::Type type_;
  uint16_t index_;

  bool process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_ylyk01yl
}  // namespace esphome
