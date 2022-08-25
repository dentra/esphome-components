#pragma once

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace crash_info {

class CrashInfo : public Component {
 public:
  CrashInfo();
  float get_setup_priority() const override { return setup_priority::LATE; }
  void setup() override;
  void dump_config() override;

  void set_indicator(binary_sensor::BinarySensor *indicator) { this->indicator_ = indicator; }

  void reset();

  void save_crash_info(uint8_t reason, uint8_t exccause, const uint32_t *stack, size_t stack_size);

  uint32_t get_max_stack_frames_size() const;
  uint32_t get_min_stack_frames_addr() const;
  uint32_t get_max_stack_frames_addr() const;

 protected:
  ESPPreferenceObject rtc_;
  binary_sensor::BinarySensor *indicator_{};
};

}  // namespace crash_info
}  // namespace esphome
