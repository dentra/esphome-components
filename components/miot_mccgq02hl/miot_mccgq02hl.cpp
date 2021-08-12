#ifdef ARDUINO_ARCH_ESP32

#include "inttypes.h"
#include "esphome/core/log.h"
#include "miot_mccgq02hl.h"

namespace esphome {
namespace miot_mccgq02hl {

static const char *TAG = "miot_mccgq02hl";

void MiotMCCGQ02HL::dump_config() {
  this->dump_config_(TAG);
  LOG_BINARY_SENSOR("  ", "Door/Window Open", this);
  LOG_BINARY_SENSOR("  ", "Light", this->light_);
  LOG_BINARY_SENSOR("  ", "Alert", this->alert_);
}

void MiotMCCGQ02HL::process_door_sensor_(const miot::BLEObject &obj) {
  auto opening = obj.get_uint8();
  if (!opening.has_value()) {
    return;
  }
  ESP_LOGD(TAG, "Opening: %" PRIu8, *opening);
  switch (*opening) {
    case 0x01: {  // closed
      this->publish_state(false);
      if (this->alert_ != nullptr) {
        this->alert_->publish_state(false);
      }
      break;
    }
    case 0x00: {  // opened
      this->publish_state(true);
      if (this->alert_ != nullptr) {
        this->alert_->publish_state(false);
      }
      break;
    }
    case 0x02: {  // not closed over time
      this->publish_state(true);
      if (this->alert_ != nullptr) {
        this->alert_->publish_state(true);
      }
      break;
    }
    case 0x03:  // device reset
      // do nothing
      break;
  }
}

void MiotMCCGQ02HL::process_light_intensity_(const miot::BLEObject &obj) {
  if (this->light_ != nullptr) {
    auto light = obj.get_bool();
    if (light.has_value()) {
      ESP_LOGD(TAG, "Light: %s", YESNO(*light));
      this->light_->publish_state(*light);
    }
  }
}

void MiotMCCGQ02HL::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_DOOR_SENSOR:
      this->process_door_sensor_(obj);
      break;

    case miot::MIID_LIGHT_INTENSITY:
      this->process_light_intensity_(obj);
      break;

    default:
      this->process_default_(obj);
      break;
  }
}

}  // namespace miot_mccgq02hl
}  // namespace esphome

#endif
