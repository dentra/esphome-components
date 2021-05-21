#pragma once

#include "esphome/core/component.h"

#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace energy_monitoring {

using sensor::Sensor;

class EnergyMonitoring : public Component {
 public:
  float get_setup_priority() const override { return esphome::setup_priority::DATA; }

  void dump_config() override;
  void setup() override;
  void loop() override;

  void set_power(Sensor *sensor) { this->power_ = sensor; }
  void set_voltage(Sensor *sensor) { this->voltage_ = sensor; }
  void set_current(Sensor *sensor) { this->current_ = sensor; }

  void set_apparent_power(Sensor *sensor) { this->apparent_power_ = sensor; }
  void set_reactive_power(Sensor *sensor) { this->reactive_power_ = sensor; }
  void set_power_factor(Sensor *sensor) { this->power_factor_ = sensor; }

  void set_wait_time(uint32_t value) { this->wait_time_ = value; }

 protected:
  // input sensors
  Sensor *power_{nullptr};
  Sensor *voltage_{nullptr};
  Sensor *current_{nullptr};

  // exposed sensors
  Sensor *apparent_power_{nullptr};
  Sensor *reactive_power_{nullptr};
  Sensor *power_factor_{nullptr};

  float get_voltage_();
  float get_current_();

  void process_power_(float power, float apparent, float current);
  void process_apparent_(float voltage, float current);

  float calc_apparent_power_(float power, float voltage, float current);
  float calc_reactive_power_(float power, float apparent, float current);
  float calc_power_factor_(float power, float apparent);

  uint32_t wait_start_{0};
  uint32_t wait_time_{0};
};

}  // namespace energy_monitoring
}  // namespace esphome
