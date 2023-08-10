#include "esphome/core/log.h"
#include "miot_mjyd02yla.h"

namespace esphome {
namespace miot_mjyd02yla {

static const char *const TAG = "miot_mjyd02yla";

constexpr uint32_t LIGHT_ILLUMINANCE = 0x64;

void MiotMJYD02YLA::dump_config() {
  this->dump_config_(TAG, "MJYD02YL-A");
  LOG_BINARY_SENSOR("  ", "Motion", this);
  LOG_BINARY_SENSOR("  ", "Light", this->light_);
  LOG_SENSOR("  ", "Idle Time", this->idle_time_);
}

void MiotMJYD02YLA::process_idle_time_(const miot::BLEObject &obj) {
  const auto idle_time = obj.get_idle_time();
  if (idle_time.has_value()) {
    this->publish_state(*idle_time == 0);
    if (this->idle_time_ != nullptr) {
      this->idle_time_->publish_state(*idle_time);
    }
  }
}

void MiotMJYD02YLA::process_motion_with_light_event_(const miot::BLEObject &obj) {
  const auto illuminance = obj.get_motion_with_light_event();
  if (illuminance.has_value()) {
    this->publish_state(true);
    if (this->light_ != nullptr) {
      this->light_->publish_state(*illuminance == LIGHT_ILLUMINANCE);
    }
  }
}

void MiotMJYD02YLA::process_illuminance_(const miot::BLEObject &obj) {
  if (this->light_ != nullptr) {
    const auto illuminance = obj.get_illuminance();
    if (illuminance.has_value()) {
      this->light_->publish_state(*illuminance == LIGHT_ILLUMINANCE);
    }
  }
}

bool MiotMJYD02YLA::process_object_(const miot::BLEObject &obj) {
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

    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace miot_mjyd02yla
}  // namespace esphome
