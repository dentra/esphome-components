#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/log.h"
#include "miot_rtcgq2lm.h"

namespace esphome {
namespace miot_rtcgq2lm {

static const char *TAG = "miot_rtcgq2lm";

void MiotRTCGQ2LM::dump_config() {
  this->dump_config_(TAG);
  LOG_BINARY_SENSOR("  ", "Motion Sensor", this);
  LOG_BINARY_SENSOR("  ", "Light", this->light_);
  LOG_BINARY_SENSOR("  ", "Timeout", this->timeout_);
  LOG_SENSOR("  ", "Idle Time", this->idle_time_);
  LOG_SENSOR("  ", "Illuminance", this->illuminance_);
}

void MiotRTCGQ2LM::process_idle_time_(const miot::BLEObject &obj) {
  auto idle_time = obj.get_uint32();
  if (idle_time.has_value()) {
    ESP_LOGD(TAG, "Idle time: %d s", *idle_time);
    if (this->idle_time_ != nullptr) {
      this->idle_time_->publish_state(*idle_time);
    }
    this->publish_state(*idle_time == 0);
    if (this->timeout_ != nullptr) {
      this->timeout_->publish_state(*idle_time != 0);
    }
  }
}

void MiotRTCGQ2LM::process_someone_is_moving_with_light_event_(const miot::BLEObject &obj) {
  auto illuminance = obj.get_uint24();
  if (illuminance.has_value()) {
    ESP_LOGD(TAG, "Motion with light: %d lx", *illuminance);
    this->publish_state(true);
    if (this->illuminance_ != nullptr) {
      this->illuminance_->publish_state(*illuminance);
    }
    if (this->light_ != nullptr) {
      this->light_->publish_state(*illuminance == 0x100);
    }
    if (this->timeout_ != nullptr) {
      this->timeout_->publish_state(false);
    }
  }
}

void MiotRTCGQ2LM::process_timeout_(const miot::BLEObject &obj) {
  auto timeout = obj.get_uint32();
  if (timeout.has_value()) {
    ESP_LOGD(TAG, "Timeout: %d s", *timeout);
    this->publish_state(false);
    if (this->timeout_ != nullptr) {
      this->timeout_->publish_state(*timeout != 0);
    }
  }
}

void MiotRTCGQ2LM::process_light_intensity_(const miot::BLEObject &obj) {
  if (this->light_ != nullptr) {
    auto light = obj.get_bool();
    if (light.has_value()) {
      ESP_LOGD(TAG, "Light: %s", YESNO(*light));
      this->light_->publish_state(*light);
    }
  }
}

void MiotRTCGQ2LM::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_IDLE_TIME:
      this->process_idle_time_(obj);
      break;

    case miot::MIID_SOMEONE_IS_MOVING_WITH_LIGHT_EVENT:
      this->process_someone_is_moving_with_light_event_(obj);
      break;

    case miot::MIID_TIMEOUT:
      this->process_timeout_(obj);
      break;

    case miot::MIID_LIGHT_INTENSITY:
      this->process_light_intensity_(obj);
      break;

    default:
      this->process_default_(obj);
      break;
  }
}

}  // namespace miot_rtcgq2lm
}  // namespace esphome

#endif
