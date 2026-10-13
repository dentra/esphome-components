#pragma once

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/time/real_time_clock.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

#include "energy_tariff.h"

namespace esphome {
namespace energy_tariffs {

using sensor::Sensor;

class EnergyTariffs : public Component {
 public:
  class TimeOffsetNumber : public number::Number, public Parented<EnergyTariffs> {
   public:
    void control(float value) override {
      this->parent_->rtc_.save(&value);
      this->publish_state(value);
    }
  };

  virtual ~EnergyTariffs();

  float get_setup_priority() const override { return setup_priority::DATA; }
  void dump_config() override;
  void setup() override;
  void loop() override;

  void set_time(time::RealTimeClock *time) { this->time_ = time; }
  void set_total(Sensor *sensor) { this->total_ = sensor; }

  void set_time_offset(number::Number *time_offset) { this->time_offset_ = time_offset; }

  void add_tariff(EnergyTariff *s) { this->tariffs_.push_back(s); }

  // Add holiday date, year 0 means every year.
  void add_holiday(uint8_t day, uint8_t month, uint16_t year) {
    this->holidays_.push_back(encode_date_(day, month, year));
  }
  // Day of week (1 is Sunday) used for tariff selection on holidays.
  void set_holiday_as(uint8_t day_of_week) { this->holiday_as_ = day_of_week; }
#ifdef USE_BINARY_SENSOR
  void set_holiday(binary_sensor::BinarySensor *holiday) { this->holiday_ = holiday; }
#endif

  EnergyTariff *get_current_tariff() const { return this->current_tariff_; }

  template<typename F> void add_on_tariff_callback(F &&callback) {
    if (!this->tariff_callback_) {
      this->tariff_callback_ = new CallbackManager<void(Sensor *)>();
    }
    this->tariff_callback_->add(std::forward<F>(callback));
  }

  template<typename F> void add_on_before_tariff_callback(F &&callback) {
    if (!this->before_tariff_callback_) {
      this->before_tariff_callback_ = new CallbackManager<void()>();
    }
    this->before_tariff_callback_->add(std::forward<F>(callback));
  }

 protected:
  ESPPreferenceObject rtc_;
  time::RealTimeClock *time_{};

  // input sensors
  Sensor *total_{};
  number::Number *time_offset_{};
#ifdef USE_BINARY_SENSOR
  binary_sensor::BinarySensor *holiday_{};
#endif

  std::vector<uint32_t> holidays_;
  uint8_t holiday_as_{1};

  // exposed sensors
  std::vector<EnergyTariff *> tariffs_;

  EnergyTariff *current_tariff_{};

  float last_total_{NAN};
  bool before_tariff_triggered_{};

  CallbackManager<void(Sensor *)> *tariff_callback_{};
  CallbackManager<void()> *before_tariff_callback_{};

  void process_(float total);

  EnergyTariff *get_tariff_(const ESPTime &time) const;

  bool is_holiday_(const ESPTime &time) const;

  static uint32_t encode_date_(uint8_t day, uint8_t month, uint16_t year) {
    return (uint32_t(year) << 16) | (uint32_t(month) << 8) | day;
  }
};  // class EnergyTariffs

}  // namespace energy_tariffs
}  // namespace esphome
