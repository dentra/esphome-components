#pragma once
#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/automation.h"
#include "../miot/miot.h"
#include "miot_ylkg0xyl.h"

namespace esphome {
namespace miot_ylkg0xyl {

enum MiotYLKG0XYLEvent {
  ON_SHORT_PRESS = 1,
  ON_LONG_PRESS,
  ON_ROTATE_LEFT,
  ON_ROTATE_RIGHT,
  ON_ROTATE_LEFT_PRESSED,
  ON_ROTATE_RIGHT_PRESSED,
};

class MiotYLKG0XYLTrigger : public Trigger<uint8_t>, public miot::MiotListener {
 public:
  explicit MiotYLKG0XYLTrigger(MiotYLKG0XYLEvent event) : event_(event) {}

  uint16_t get_product_id() const override { return PRODUCT_ID; }

 protected:
  MiotYLKG0XYLEvent event_;

  bool process_object_(const miot::BLEObject &obj) override {
    switch (obj.id) {
      case miot::MIID_BUTTON_EVENT: {
        const auto button_event = obj.get_button_event();
        if (button_event.has_value()) {
          return this->process_button_event_(*button_event);
        }
        return false;
      }
      default:
        return this->process_unhandled_(obj);
    }
    return false;
  }

  bool process_button_event_(const miot::ButtonEvent &button_event) {
    switch (this->event_) {
      case ON_SHORT_PRESS: {
        if (button_event.type == miot::ButtonEvent::TRIPLE_CLICK_OR_ROTATE_KNOB && button_event.index == 0) {
          this->trigger(button_event.value < 0 ? -button_event.value : button_event.value);
          return true;
        }
        break;
      }
      case ON_LONG_PRESS: {
        if (button_event.type == miot::ButtonEvent::TRIPLE_CLICK_OR_ROTATE_KNOB && button_event.index == 0) {
          this->trigger(button_event.value < 0 ? -button_event.value : button_event.value);
          return true;
        }
        break;
      }
      case ON_ROTATE_LEFT: {
        if (button_event.type == miot::ButtonEvent::ROTATE && button_event.index == 0 && button_event.value < 0) {
          this->trigger(-button_event.value);
          return true;
        }
        break;
      }
      case ON_ROTATE_RIGHT: {
        if (button_event.type == miot::ButtonEvent::ROTATE && button_event.index == 0 && button_event.value > 0) {
          this->trigger(button_event.value);
        }
        break;
      }
      case ON_ROTATE_LEFT_PRESSED: {
        int8_t dimmer = button_event.index;
        if (button_event.type == miot::ButtonEvent::ROTATE && dimmer < 0) {
          this->trigger(-dimmer);
          return true;
        }
        break;
      }
      case ON_ROTATE_RIGHT_PRESSED: {
        int8_t dimmer = button_event.index;
        if (button_event.type == miot::ButtonEvent::ROTATE && dimmer > 0) {
          this->trigger(dimmer);
          return true;
        }
        break;
      }
    }
    return false;
  }
};

}  // namespace miot_ylkg0xyl
}  // namespace esphome
#endif
