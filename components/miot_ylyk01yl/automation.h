#pragma once
#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/automation.h"
#include "../miot/miot.h"
#include "miot_ylyk01yl.h"

namespace esphome {
namespace miot_ylyk01yl {

class MiotYLYK01YLTrigger : public Trigger<>, public miot::MiotListener {
 public:
  explicit MiotYLYK01YLTrigger(miot::ButtonEvent::Type type, uint16_t index) : type_(type), index_(index) {}

  uint16_t get_product_id() const override { return PRODUCT_ID; }

 protected:
  miot::ButtonEvent::Type type_;
  uint16_t index_;
  bool process_object_(const miot::BLEObject &obj) override {
    switch (obj.id) {
      case miot::MIID_BUTTON_EVENT: {
        const auto button_event = obj.get_button_event();
        if (button_event.has_value()) {
          ESP_LOGD("miot_ylyk01yl", "Checking trigger button event: type %" PRIu8 " == %" PRIu8 " and index %u == %u",
                   this->type_, button_event->type, this->index_, button_event->index);
          if (this->type_ == button_event->type && this->index_ == button_event->index) {
            this->trigger();
            return true;
          }
        }
        return false;
      }
      case miot::MIID_SIMPLE_PAIRING_EVENT:
        // skip this event
        // do not know how to process it yet.
        return false;
      default:
        return this->process_unhandled_(obj);
    }
    return false;
  }
};

}  // namespace miot_ylyk01yl
}  // namespace esphome
#endif
