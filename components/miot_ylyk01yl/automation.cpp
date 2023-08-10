#include "esphome/core/log.h"
#include "automation.h"
namespace esphome {
namespace miot_ylyk01yl {

static const char *const TAG = "miot_ylyk01yl.automation";

bool MiotYLYK01YLTrigger::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_BUTTON_EVENT: {
      const auto button_event = obj.get_typed<miot::ButtonEvent>();
      if (button_event != nullptr) {
        if (this->type_ == button_event->type && this->index_ == button_event->button.index) {
          button_event->dump(TAG);
          this->trigger();
          return true;
        }
      }
      return false;
    }
    case miot::MIID_PAIRING_EVENT:
      // skip this event
      return false;
    default:
      return this->process_unhandled_(obj);
  }
  return false;
}
}  // namespace miot_ylyk01yl
}  // namespace esphome
