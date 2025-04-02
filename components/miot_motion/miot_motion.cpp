#include "esphome/core/log.h"
#include "miot_motion.h"

namespace esphome {
namespace miot_motion {

static const char *const TAG = "miot_motion";

constexpr uint16_t PRODUCT_ID_MJYD02YLA = 0x07F6;
constexpr uint16_t PRODUCT_ID_RTCGQ02LM = 0x0A8D;
constexpr uint16_t PRODUCT_ID_CGPR1 = 0x0A83;

void MiotMotion::dump_config() {
  this->dump_config_(TAG, "Motion");

  LOG_BINARY_SENSOR("  ", "Motion", this);
  LOG_BINARY_SENSOR("  ", "Light", this->light_binary_sensor_);
  LOG_SENSOR("  ", "Idle Time", this->idle_time_sensor_);
  LOG_SENSOR("  ", "Illuminance", this->illuminance_sensor_);

  if (this->reset_timeout_) {
    ESP_LOGCONFIG(TAG, "  Timeout: %.3f s", this->reset_timeout_ * 0.001f);
  }
}

void MiotMotion::setup() {
  if (this->reset_timeout_) {
    this->add_on_state_callback(
        [this](bool state) { this->set_timeout(TAG, this->reset_timeout_, [this]() { this->publish_state(false); }); });
  }
}

void MiotMotion::process_idle_time_(uint32_t idle_time, bool has_time) {
  // 0 = motion detected
  this->publish_state(idle_time == 0);

  if (this->idle_time_sensor_ && has_time) {
    this->idle_time_sensor_->publish_state(idle_time);
  }
}

void MiotMotion::process_idle_time_(const miot::BLEObject &obj) {
  const auto idle_time = obj.get_idle_time();
  if (idle_time.has_value()) {
    this->process_idle_time_(*idle_time, true);
  }
}

void MiotMotion::process_timeout_(const miot::BLEObject &obj) {
  const auto timeout = obj.get_timeout();
  if (timeout.has_value()) {
    this->process_idle_time_(*timeout, false);
  }
}

void MiotMotion::process_no_motion_time_(const miot::BLEObject &obj) {
  const auto idle_time = obj.get_no_motion_time();
  if (idle_time.has_value()) {
    this->process_idle_time_(*idle_time, true);
  }
}

void MiotMotion::process_motion_with_illuminance_(float illuminance) {
  // MJYD02YLA: 1 - moving no light, 100 - moving with light
  // RTCGQ02LM: 0 - moving no light, 256 - moving with light

  this->publish_state(true);

  if (this->light_binary_sensor_) {
    this->light_binary_sensor_->publish_state(illuminance >= 100);
  }

  if (this->illuminance_sensor_) {
    this->illuminance_sensor_->publish_state(illuminance);
  }
}

void MiotMotion::process_motion_with_light_event_(const miot::BLEObject &obj) {
  const auto illuminance = obj.get_motion_with_light_event();
  if (illuminance.has_value()) {
    this->process_motion_with_illuminance_(*illuminance);
  }
}

void MiotMotion::process_motion_with_illuminance_(const miot::BLEObject &obj) {
  const auto illuminance = obj.get_motion_with_illuminance();
  if (illuminance.has_value()) {
    this->process_motion_with_illuminance_(*illuminance);
  }
}

void MiotMotion::process_light_intensity_(const miot::BLEObject &obj) {
  if (this->light_binary_sensor_) {
    const auto light = obj.get_light_intensity();
    if (light.has_value()) {
      this->light_binary_sensor_->publish_state(*light);
    }
  }
}

void MiotMotion::process_illuminance_(const miot::BLEObject &obj) {
  const auto illuminance = obj.get_illuminance();
  if (!illuminance.has_value()) {
    return;
  }

  if (this->light_binary_sensor_) {
    this->light_binary_sensor_->publish_state(*illuminance >= 100);
  }

  if (this->illuminance_sensor_) {
    this->illuminance_sensor_->publish_state(*illuminance);
  }
}

void MiotMotion::process_motion_event_(const miot::BLEObject &obj) {
  const auto motion = obj.get_motion();
  if (!motion.has_value()) {
    return;
  }
  this->publish_state(*motion);
}

bool MiotMotion::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    // RTCGQ02LM, MJYD02YLA, CGPR1
    case miot::MIID_IDLE_TIME:
      this->process_idle_time_(obj);
      break;

    // RTCGQ02LM
    case miot::MIID_TIMEOUT:
      this->process_timeout_(obj);
      break;

    case miot::MIID_NO_MOTION_TIME:
      this->process_no_motion_time_(obj);
      break;

    case miot::MIID_MOTION_EVENT:
      this->process_motion_event_(obj);
      break;

    // RTCGQ02LM, MJYD02YLA
    case miot::MIID_MOTION_WITH_LIGHT_EVENT:
      this->process_motion_with_light_event_(obj);
      break;

    case miot::MIID_MOTION_WITH_ILLUMINANCE:
      this->process_motion_with_illuminance_(obj);
      break;

    // RTCGQ02LM
    case miot::MIID_LIGHT_INTENSITY:
      this->process_light_intensity_(obj);
      break;

    // MJYD02YLA, CGPR1
    case miot::MIID_ILLUMINANCE:
      this->process_illuminance_(obj);
      break;

    default:
      return this->process_default_(obj);
  }

  return true;
}

}  // namespace miot_motion
}  // namespace esphome
