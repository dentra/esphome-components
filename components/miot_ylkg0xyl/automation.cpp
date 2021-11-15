#include "esphome/core/log.h"
#include "automation.h"

namespace esphome {
namespace miot_ylkg0xyl {
static const char *const TAG = "miot_ylkg0xyl.automation";

bool MiotYLKG0XYLTrigger::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_BUTTON_EVENT: {
      const auto button_event = obj.get_typed<miot::ButtonEvent>();
      if (button_event != nullptr) {
        uint8_t value = this->process_button_event_(*button_event);
        if (value != 0) {
          miot::ButtonEvent::dump(TAG, *button_event);
          this->trigger(value);
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

bool MiotYLKG0XYLTrigger::process_button_event_(const miot::ButtonEvent &button_event) {
  switch (this->event_) {
    case ON_SHORT_PRESS: {
      if (button_event.type == miot::ButtonEvent::KNOB) {
        return button_event.knob.short_press();
      }
      break;
    }
    case ON_LONG_PRESS: {
      if (button_event.type == miot::ButtonEvent::KNOB) {
        return button_event.knob.long_press();
      }
      break;
    }
    case ON_ROTATE_LEFT: {
      if (button_event.type == miot::ButtonEvent::DIMMER) {
        return button_event.dimmer.left();
      }
      break;
    }
    case ON_ROTATE_RIGHT: {
      if (button_event.type == miot::ButtonEvent::DIMMER) {
        return button_event.dimmer.right();
      }
      break;
    }
    case ON_ROTATE_LEFT_PRESSED: {
      if (button_event.type == miot::ButtonEvent::DIMMER) {
        return button_event.dimmer.left_pressed();
      }
      break;
    }
    case ON_ROTATE_RIGHT_PRESSED: {
      if (button_event.type == miot::ButtonEvent::DIMMER) {
        return button_event.dimmer.right_pressed();
      }
      break;
    }
  }
  return 0;
}

}  // namespace miot_ylkg0xyl
}  // namespace esphome
