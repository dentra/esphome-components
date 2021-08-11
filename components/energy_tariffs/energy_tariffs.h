#pragma once

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/time/real_time_clock.h"

#ifdef USE_API
#include "esphome/components/api/custom_api_device.h"
#endif

#include "energy_tariff.h"

namespace esphome {
namespace energy_tariffs {

using sensor::Sensor;

class EnergyTariffs : public Component
#ifdef USE_API
    ,
                      public api::CustomAPIDevice
#endif
{
 public:
  virtual ~EnergyTariffs();

  float get_setup_priority() const override { return setup_priority::DATA; }
  void dump_config() override;
  void setup() override;
  void loop() override;

  void set_time(time::RealTimeClock *time) { this->time_ = time; }
  void set_total(Sensor *sensor) { this->total_ = sensor; }

  void set_time_offset(int seconds) { this->time_offset_ = seconds; }
  int get_time_offset() const { return this->time_offset_; }
  void set_time_offset_service(const std::string &service) { this->time_offset_service_ = service; }

  void add_tariff(EnergyTariff *s) { this->tariffs_.push_back(s); }

  EnergyTariff *get_current_tariff() const { return this->current_tariff_; }

  void add_on_tariff_callback(std::function<void(Sensor *)> &&callback);
  void add_on_before_tariff_callback(std::function<void()> &&callback);

 protected:
  time::RealTimeClock *time_;
  int time_offset_{0};

  // input sensors
  Sensor *total_{nullptr};

  // exposed sensors
  std::vector<EnergyTariff *> tariffs_;

  EnergyTariff *current_tariff_{nullptr};

  float last_total_{0};

  CallbackManager<void(Sensor *)> *tariff_callback_{nullptr};
  CallbackManager<void()> *before_tariff_callback_{nullptr};

  void process_(float total);

  EnergyTariff *get_tariff_(const time::ESPTime &time) const;

#ifdef USE_API
  ESPPreferenceObject rtc_;
  std::string time_offset_service_;
  // set time offset in seconds
  void set_time_offset_(float value);
#endif
};  // class EnergyTariffs

}  // namespace energy_tariffs
}  // namespace esphome
