#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/log.h"
#include "miot_cgpr1.h"

namespace esphome {
namespace miot_cgpr1 {

static const char *const TAG = "miot_cgpr1";

void MiotCGPR1::dump_config() {
  this->dump_config_(TAG);
  LOG_BINARY_SENSOR("  ", "Motion", this);
  LOG_BINARY_SENSOR("  ", "Light", this->light_);
  LOG_SENSOR("  ", "Idle Time", this->idle_time_);
  LOG_SENSOR("  ", "Illuminance", this->illuminance_);
}

void MiotCGPR1::process_idle_time_(const miot::BLEObject &obj) {
  const auto idle_time = obj.get_idle_time();
  if (idle_time.has_value()) {
    this->publish_state(*idle_time == 0);
    if (this->idle_time_ != nullptr) {
      this->idle_time_->publish_state(*idle_time);
    }
  }
}

void MiotCGPR1::process_motion_with_light_event_(const miot::BLEObject &obj) {
  const auto illuminance = obj.get_motion_with_light_event();
  if (illuminance.has_value()) {
    this->publish_state(true);
    if (this->illuminance_ != nullptr) {
      this->illuminance_->publish_state(*illuminance);
    }
  }
}

void MiotCGPR1::process_illuminance_(const miot::BLEObject &obj) {
  if (this->illuminance_ != nullptr) {
    const auto illuminance = obj.get_illuminance();
    if (illuminance.has_value()) {
      this->illuminance_->publish_state(*illuminance);
    }
  }
}

void MiotCGPR1::process_light_intensity_(const miot::BLEObject &obj) {
  if (this->light_ != nullptr) {
    const auto light = obj.get_light_intensity();
    if (light.has_value()) {
      this->light_->publish_state(*light);
    }
  }
}

bool MiotCGPR1::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_IDLE_TIME:
      this->process_idle_time_(obj);
      break;

    case miot::MIID_MOTION_WITH_LIGHT_EVENT:
      this->process_motion_with_light_event_(obj);
      break;

    case miot::MIID_ILLUMINANCE:
      this->process_illuminance_(obj);
      break;

    case miot::MIID_LIGHT_INTENSITY:
      this->process_light_intensity_(obj);
      break;

    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace miot_cgpr1
}  // namespace esphome

#endif
