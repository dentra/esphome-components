#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace zmai90v1 {

struct zmai90_data_t {
  uint8_t header[3];          // FE0108
  uint8_t energy[4];          // kWh, divided by 100
  uint8_t voltage[4];         // V, divided by 10
  uint8_t current[4];         // A, divided by 10000
  uint8_t frequency[4];       // Hz, divided by 100
  uint8_t active_power[4];    // W, divided by 100
  uint8_t reactive_power[4];  // divided by 100
  uint8_t apparent_power[4];  // divided by 100
  uint8_t power_factor[4];    // %, divided by 10
  uint8_t checksum;
} PACKED;

enum ZMAi90v1RestoreMode {
  RESTORE_MODE_ALWAYS_ON,
  RESTORE_MODE_ALWAYS_OFF,
  RESTORE_MODE_RESTORE_DEFAULT_ON,
  RESTORE_MODE_RESTORE_DEFAULT_OFF,
};

class ZMAi90v1 : public PollingComponent, public switch_::Switch, public uart::UARTDevice {
 public:
  void dump_config() override;
  void setup() override;
  void loop() override;
  void update() override;

  void set_restore_mode(ZMAi90v1RestoreMode restore_mode) { this->restore_mode_ = restore_mode; }

  void set_switch_pin(GPIOPin *pin) { switch_pin_ = pin; }
  void set_button_pin(GPIOPin *pin) { button_pin_ = pin; }
  void set_button(binary_sensor::BinarySensor *button) { button_ = button; }

  void set_energy(sensor::Sensor *value) { this->energy_ = value; }
  void set_voltage(sensor::Sensor *value) { this->voltage_ = value; }
  void set_current(sensor::Sensor *value) { this->current_ = value; }
  void set_active_power(sensor::Sensor *value) { this->active_power_ = value; }
  void set_reactive_power(sensor::Sensor *value) { this->reactive_power_ = value; }
  void set_apparent_power(sensor::Sensor *value) { this->apparent_power_ = value; }
  void set_frequency(sensor::Sensor *value) { this->frequency_ = value; }
  void set_power_factor(sensor::Sensor *value) { this->power_factor_ = value; }

 protected:
  ZMAi90v1RestoreMode restore_mode_{RESTORE_MODE_ALWAYS_ON};
  GPIOPin *switch_pin_ = {};
  GPIOPin *button_pin_ = {};
  binary_sensor::BinarySensor *button_ = {};
  sensor::Sensor *energy_ = {};
  sensor::Sensor *voltage_ = {};
  sensor::Sensor *current_ = {};
  sensor::Sensor *active_power_ = {};
  sensor::Sensor *reactive_power_ = {};
  sensor::Sensor *apparent_power_ = {};
  sensor::Sensor *frequency_ = {};
  sensor::Sensor *power_factor_ = {};

  float get_val(const uint8_t data[4], float mul);
  uint8_t calc_crc_(const zmai90_data_t &data);

  void write_state(bool state) override;
};

}  // namespace zmai90v1
}  // namespace esphome
