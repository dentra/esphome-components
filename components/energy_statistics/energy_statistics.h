#pragma once

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/time/real_time_clock.h"

namespace esphome {
namespace energy_statistics {

using sensor::Sensor;

class EnergyStatistics : public Component {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void dump_config() override;
  void setup() override;
  void loop() override;

  void set_time(time::RealTimeClock *time) { this->time_ = time; }
  void set_total(Sensor *sensor) { this->total_ = sensor; }

  void set_energy_today(Sensor *sensor) { this->energy_today_ = sensor; }
  void set_energy_yesterday(Sensor *sensor) { this->energy_yesterday_ = sensor; }
  void set_energy_week(Sensor *sensor) { this->energy_week_ = sensor; }
  void set_energy_month(Sensor *sensor) { this->energy_month_ = sensor; }
  void set_energy_year(Sensor *sensor) { this->energy_year_ = sensor; }

 protected:
  ESPPreferenceObject pref_;
  time::RealTimeClock *time_;

  // input sensors
  Sensor *total_{nullptr};

  // exposed sensors
  Sensor *energy_today_{nullptr};
  Sensor *energy_yesterday_{nullptr};
  Sensor *energy_week_{nullptr};
  Sensor *energy_month_{nullptr};
  Sensor *energy_year_{nullptr};

  // start day of week configuration
  int energy_week_start_day_{2};
  // start day of month configuration
  int energy_month_start_day_{1};
  // start day of year configuration
  int energy_year_start_day_{1};

  struct energy_data_v1_t {
    uint16_t current_day_of_year{0};
    float start_today{NAN};
    float start_yesterday{NAN};
    float start_week{NAN};
    float start_month{NAN};
  };

  struct energy_data_t : public energy_data_v1_t {
    float start_year{NAN};
  } energy_;

  void process_(float total);
};  // class EnergyStatistics

}  // namespace energy_statistics
}  // namespace esphome
