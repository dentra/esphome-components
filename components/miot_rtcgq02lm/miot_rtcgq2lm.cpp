#include "esphome/core/log.h"
#include "miot_rtcgq2lm.h"

namespace esphome {
namespace miot_rtcgq02lm {

static const char *const TAG = "miot_rtcgq2lm";

void MiotRTCGQ02LM::dump_config() {
  this->dump_config_(TAG);
  LOG_BINARY_SENSOR("  ", "Motion", this);
  LOG_BINARY_SENSOR("  ", "Light", this->light_);
  LOG_BINARY_SENSOR("  ", "Timeout", this->timeout_);
  LOG_SENSOR("  ", "Idle Time", this->idle_time_);
  // LOG_SENSOR("  ", "Illuminance", this->illuminance_);
}

void MiotRTCGQ02LM::process_idle_time_(const miot::BLEObject &obj) {
  const auto idle_time = obj.get_idle_time();
  if (idle_time.has_value()) {
    this->publish_state(*idle_time == 0);
    if (this->idle_time_ != nullptr) {
      this->idle_time_->publish_state(*idle_time);
    }
    if (this->timeout_ != nullptr) {
      this->timeout_->publish_state(*idle_time != 0);
    }
  }
}

void MiotRTCGQ02LM::process_motion_with_light_event_(const miot::BLEObject &obj) {
  const auto illuminance = obj.get_motion_with_light_event();
  if (illuminance.has_value()) {
    this->publish_state(true);
    // if (this->illuminance_ != nullptr) {
    //   this->illuminance_->publish_state(*illuminance);
    // }
    if (this->light_ != nullptr) {
      this->light_->publish_state(*illuminance == 0x100);
    }
    if (this->timeout_ != nullptr) {
      this->timeout_->publish_state(false);
    }
  }
}

void MiotRTCGQ02LM::process_timeout_(const miot::BLEObject &obj) {
  const auto timeout = obj.get_timeout();
  if (timeout.has_value()) {
    this->publish_state(false);
    if (this->timeout_ != nullptr) {
      this->timeout_->publish_state(*timeout != 0);
    }
  }
}

void MiotRTCGQ02LM::process_light_intensity_(const miot::BLEObject &obj) {
  if (this->light_ != nullptr) {
    const auto light = obj.get_light_intensity();
    if (light.has_value()) {
      this->light_->publish_state(*light);
    }
  }
}

bool MiotRTCGQ02LM::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_IDLE_TIME:
      this->process_idle_time_(obj);
      break;

    case miot::MIID_MOTION_WITH_LIGHT_EVENT:
      this->process_motion_with_light_event_(obj);
      break;

    case miot::MIID_TIMEOUT:
      this->process_timeout_(obj);
      break;

    case miot::MIID_LIGHT_INTENSITY:
      this->process_light_intensity_(obj);
      break;

    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace miot_rtcgq02lm
}  // namespace esphome
