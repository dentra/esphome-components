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

  // Days of week bitmask, bit 1 is Sunday and bit 7 is Saturday (see ESPTime::day_of_week).
  void set_days_of_week(uint8_t days_of_week) { this->days_of_week_ = days_of_week; }

  void set_service(const std::string &service) { this->service_ = service; }
  void set_initial_value(float initial_value) { this->initial_value_ = initial_value; }

  bool is_default() const { return this->time_.empty() && this->days_of_week_ == 0; }

  // Return true if tariff is active at specified time. Day of week is passed separately
  // because holidays may override it.
  bool is_active(const ESPTime &time, uint8_t day_of_week) const {
    if (this->days_of_week_ != 0 && (this->days_of_week_ & (1 << day_of_week)) == 0) {
      return false;
    }
    if (this->time_.empty()) {
      return true;
    }
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
  uint8_t days_of_week_{};
  std::string service_;
  float initial_value_{0.0f};

  // Return true if time in the range [min, max]
  static bool time_in_range_(uint16_t min, uint16_t max, const ESPTime &time) {
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
};  // class TariffSensor

}  // namespace energy_tariffs
}  // namespace esphome
