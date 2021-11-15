#include "esphome/core/log.h"
#include "automation.h"

namespace esphome {
namespace miot_xmmfo1jqd {

static const char *const TAG = "miot_xmmfo1jqd.automation";

bool MiotXMMFO1JQDTrigger::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_BUTTON_EVENT: {
      const auto button_event = obj.get_typed<miot::ButtonEvent>();
      if (button_event != nullptr) {
        if (this->type_ == button_event->type && this->index_ == button_event->button.index) {
          miot::ButtonEvent::dump(TAG, *button_event);
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

}  // namespace miot_xmmfo1jqd
}  // namespace esphome
