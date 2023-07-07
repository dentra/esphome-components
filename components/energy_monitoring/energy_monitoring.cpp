#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "energy_monitoring.h"

#ifndef isnan
using std::isnan;
#endif

namespace esphome {
namespace energy_monitoring {

static const char *const TAG = "energy_monitoring";
static const char *const GAP = "  ";

void EnergyMonitoring::dump_config() {
  ESP_LOGCONFIG(TAG, "Energy monitoring sensors");
  LOG_SENSOR(GAP, "Apparent power", this->apparent_power_);
  LOG_SENSOR(GAP, "Reactive power", this->reactive_power_);
  LOG_SENSOR(GAP, "Power factor", this->power_factor_);
}

void EnergyMonitoring::setup() {
  // wait this->wait_time_ miliseconds to try to reduce repeative publishing
  // when input sensors report their values at same time
  if (this->wait_time_) {
    auto cb = [this](float) {
      if (!this->wait_start_) {
        this->wait_start_ = millis();
      }
    };
    this->power_->add_on_state_callback(cb);
    this->voltage_->add_on_state_callback(cb);
    if (this->current_) {
      this->current_->add_on_state_callback(cb);
    }
  } else {
    // calculates reactive power and
    // power factor
    this->power_->add_on_state_callback([this](float state) {
      auto apparent = this->apparent_power_->get_state();
      auto current = this->get_current_();
      this->process_power_(state, apparent, current);
    });

    // calculates apparent power
    // and notify to change
    // reactive power and power factor
    this->voltage_->add_on_state_callback([this](float state) {
      auto current = this->get_current_();
      this->process_apparent_(state, current);
    });

    // calculates apparent power
    // and notify to change
    // reactive power and power factor
    if (this->current_) {
      this->current_->add_on_state_callback([this](float state) {
        auto voltage = this->get_voltage_();
        this->process_apparent_(voltage, state);
      });
    }
  }

  if (this->power_factor_) {
    auto power_factor_unit = this->power_factor_->get_unit_of_measurement();
    if (!power_factor_unit.empty() && power_factor_unit[0] == '%') {
      this->power_factor_->set_accuracy_decimals(0);
    }
  }
}

void EnergyMonitoring::loop() {
  if (this->wait_start_ && (millis() - this->wait_start_) > this->wait_time_) {
    this->wait_start_ = 0;

    auto voltage = this->get_voltage_();
    auto current = this->get_current_();
    this->process_apparent_(voltage, current);
  }
}

inline float EnergyMonitoring::get_voltage_() { return this->voltage_->get_state(); }

inline float EnergyMonitoring::get_current_() {
  if (this->current_) {
    return this->current_->get_state();
  }
  return this->power_->get_state() / this->get_voltage_();
}

// process apparent power change.
// publish state to sensors:
//   - apparent power
//   - reactive power
//   - power factor
void EnergyMonitoring::process_apparent_(float voltage, float current) {
  if (std::isnan(voltage)) {
    return;
  }
  if (std::isnan(current)) {
    return;
  }
  auto power = this->power_->get_state();
  auto apparent = this->calc_apparent_power_(power, voltage, current);
  this->apparent_power_->publish_state(apparent);
  this->process_power_(power, apparent, current);
}

// process power change.
// publish state to sendors:
//   - reactive power
//   - power factor
void EnergyMonitoring::process_power_(float power, float apparent, float current) {
  if (std::isnan(apparent)) {
    return;
  }

  if (this->reactive_power_) {
    auto reactive = this->calc_reactive_power_(power, apparent, current);
    this->reactive_power_->publish_state(reactive);
  }

  if (this->power_factor_) {
    auto factor = this->calc_power_factor_(power, apparent);
    auto power_factor_unit = this->power_factor_->get_unit_of_measurement();
    if (!power_factor_unit.empty() && power_factor_unit[0] == '%') {
      factor *= 100;
    }
    this->power_factor_->publish_state(factor);
  }
}

// calculating reactive power only if current is greater than 0.005A and
// difference between active and apparent power is greater than 1.5W or 1%
//
// condition borrowed from tasmota
inline bool should_calc_reactive_power(float power, float apparent, float current) {
  // condition with NAN is always return false, so check for positive and negate
  if (!(current > 0.005)) {
    return false;
  }
  uint32_t difference = ((uint32_t) (apparent * 100) - (uint32_t) (power * 100)) / 10;
  return (difference > 15) || (difference > (uint32_t) (apparent * 100 / 1000));
}

// Reactive Power (VAr)
// Реактивная мощность
// power (p) - active power
// apparent (s) - apparent power
// current (i) - current
// q = sqrt(s * s - p * p)
inline float EnergyMonitoring::calc_reactive_power_(float power, float apparent, float current) {
  if (should_calc_reactive_power(power, apparent, current)) {
    return std::sqrt(apparent * apparent - power * power);
  }
  return 0.0;
}

// Apparent Power (VA)
// Полная мощность
// power (p) - active power
// volatage (u) - volatage
// current (i) - current
// s = u * i
// s = sqrt(p * p + q * q)
inline float EnergyMonitoring::calc_apparent_power_(float power, float voltage, float current) {
  auto apparent = voltage * current;
  // condition borrowed from tasmota
  if (apparent < power) {
    // should be impossible
    return power;
  }
  return apparent;
}

// Power Factor (Cos φ)
// Коэффициент мощности
// power (p) - active power
// apparent (s) - apparent power
// cos_fi = p / s
inline float EnergyMonitoring::calc_power_factor_(float power, float apparent) {
  if (power > 0.0 && apparent > 0.0) {
    float pf = power / apparent;
    return pf > 1.0 ? 1.0 : pf;
  }
  return 0.0;
}

}  // namespace energy_monitoring
}  // namespace esphome
