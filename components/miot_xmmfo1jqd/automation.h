#pragma once
#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/automation.h"
#include "../miot/miot.h"
#include "miot_xmmfo1jqd.h"

namespace esphome {
namespace miot_xmmfo1jqd {

class MiotXMMFO1JQDTrigger : public Trigger<>, public miot::MiotListener {
 public:
  explicit MiotXMMFO1JQDTrigger(miot::ButtonEvent::Type type, uint16_t index) : type_(type), index_(index) {}

  uint16_t get_product_id() const override { return PRODUCT_ID; }

 protected:
  miot::ButtonEvent::Type type_;
  uint16_t index_;
  bool process_object_(const miot::BLEObject &obj) override {
    switch (obj.id) {
      case miot::MIID_BUTTON_EVENT: {
        const auto button_event = obj.get_button_event();
        if (button_event.has_value()) {
          if (this->type_ == button_event->type && this->index_ == button_event->index) {
            this->trigger();
            return true;
          }
        }
        return false;
      }
      default:
        return this->process_unhandled_(obj);
    }
    return false;
  }
};

}  // namespace miot_xmmfo1jqd
}  // namespace esphome
#endif
