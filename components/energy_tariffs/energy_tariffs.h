#pragma once

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/time/real_time_clock.h"

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

  EnergyTariff *get_current_tariff() const { return this->current_tariff_; }

  void add_on_tariff_callback(std::function<void(Sensor *)> &&callback);
  void add_on_before_tariff_callback(std::function<void()> &&callback);

 protected:
  ESPPreferenceObject rtc_;
  time::RealTimeClock *time_{};

  // input sensors
  Sensor *total_{};
  number::Number *time_offset_{};

  // exposed sensors
  std::vector<EnergyTariff *> tariffs_;

  EnergyTariff *current_tariff_{};

  float last_total_{NAN};

  CallbackManager<void(Sensor *)> *tariff_callback_{};
  CallbackManager<void()> *before_tariff_callback_{};

  void process_(float total);

  EnergyTariff *get_tariff_(const time::ESPTime &time) const;
};  // class EnergyTariffs

}  // namespace energy_tariffs
}  // namespace esphome
