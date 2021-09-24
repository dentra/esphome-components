#ifdef ARDUINO_ARCH_ESP32
#include "esphome/core/log.h"
#include "automation.h"

namespace esphome {
namespace miot_ylai003 {

static const char *const TAG = "miot_ylai003.automation";

bool MiotYLAI003Trigger::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_BUTTON_EVENT: {
      const auto button_event = obj.get_button_event();
      if (button_event.has_value()) {
        if (button_event->index != 0) {
          ESP_LOGW(TAG, "Unknown button index detected: %u", button_event->index);
        }
        if (this->type_ == button_event->type) {
          this->trigger();
          return true;
        }
      }
      return false;
    }
    case miot::MIID_SIMPLE_PAIRING_EVENT:
      // skip pairing event
      return false;
    default:
      return this->process_unhandled_(obj);
  }
  return false;
}

}  // namespace miot_ylai003
}  // namespace esphome

#endif
