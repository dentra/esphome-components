#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/time/real_time_clock.h"

#ifdef USE_API
#include "esphome/components/api/custom_api_device.h"
#endif

namespace esphome {
namespace energy_tariffs {

using sensor::Sensor;

// Time range.
// In this customisation we assume that time is a minutes count of the day and calculated by
// formula: hour * 60 + minute.
struct time_range_t {
  uint16_t min;
  uint16_t max;
};

class EnergyTariff : public Sensor,
                     public Component
#ifdef USE_API
    ,
                     public api::CustomAPIDevice
#endif
{
 public:
  void dump_config() override;
  void setup() override;

  void add_time(uint16_t min, uint16_t max) {
    this->time_.push_back({
        .min = min,
        .max = max,
    });
  }

  void set_service(const std::string &x) { this->service = x; }
  void set_save_to_flash_interval(uint32_t save_to_flash_interval) {
    this->save_to_flash_interval_ = save_to_flash_interval;
  }

  bool is_default() const { return this->time_.empty(); }

  bool time_in_range(const time::ESPTime &time) const {
    for (auto const &range : this->time_) {
      if (time_in_range_(range.min, range.max, time)) {
        return true;
      }
    }
    return false;
  }

  void add(float x) { this->publish_state_and_save(x + this->state); }

  void publish_state_and_save(float state);

 protected:
  ESPPreferenceObject rtc_;
  std::vector<time_range_t> time_;
  std::string service;

  // Return true if time in the range [min, max]
  static bool time_in_range_(uint16_t min, uint16_t max, const time::ESPTime &time) {
    auto x = time.hour * 60 + time.minute;
    return time_in_range_(min, max, x);
  }

  // Return true if x is in the range [min, max].
  static bool time_in_range_(uint16_t min, uint16_t max, uint16_t x) {
    if (min <= max) {
      return min <= x && x < max;
    }
    return min <= x || x < max;
  }

  uint32_t last_save_{0};
  uint32_t save_to_flash_interval_{};

};  // class TariffSensor

}  // namespace energy_tariffs
}  // namespace esphome
