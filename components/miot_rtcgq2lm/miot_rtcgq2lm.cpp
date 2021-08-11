#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/log.h"
#include "miot_rtcgq2lm.h"

namespace esphome {
namespace miot_rtcgq2lm {

static const char *TAG = "miot_rtcgq2lm";

void MiotRTCGQ2LM::dump_config() {
  this->dump_config_(TAG);
  LOG_BINARY_SENSOR("  ", "Motion Sensor", this);
}

void MiotRTCGQ2LM::process_object_(const miot::BLEObject &obj) {
  auto idle_time = obj.get_uint32(miot::ATTR_IDLE_TIME);  // 1017
  if (idle_time.has_value()) {
    ESP_LOGD(TAG, "  Idle time: %d s", *idle_time);
    if (this->idle_time_ != nullptr) {
      this->idle_time_->publish_state(*idle_time);
    }
    this->publish_state(*idle_time == 0);
  }
  auto illuminance = obj.get_uint32(miot::ATTR_PC_SOMEONE_IS_MOVING_WITH_LIGHT);  // 000F
  if (illuminance.has_value()) {
    ESP_LOGD(TAG, "  Motion with light: %d lx", *illuminance);
    this->publish_state(true);
    if (this->illuminance_ != nullptr) {
      this->illuminance_->publish_state(*illuminance);
    }
    if (this->light_ != nullptr) {
      this->light_->publish_state(*illuminance == 0x100);
    }
  }
  auto no_moves = obj.get_uint32(miot::ATTR_NO_MOVES);  // 101B
  if (no_moves.has_value()) {
    ESP_LOGD(TAG, "  No moves: %d s", *no_moves);
    this->publish_state(false);
  }
  if (this->light_ != nullptr) {
    auto light = obj.get_bool(miot::ATTR_LIGHT_INTENSITY);  // 1018
    if (light.has_value()) {
      ESP_LOGD(TAG, "  Light: %s", YESNO(*light));
      this->light_->publish_state(*light);
    }
  }
}

}  // namespace miot_rtcgq2lm
}  // namespace esphome

#endif
