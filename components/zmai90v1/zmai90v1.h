#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace zmai90v1 {

struct zmai90_data_t {
  template<size_t DIV> struct field_t {
    uint8_t data[4];
    float value() { return value(1.0f / DIV); }
    float value(float mul) {
      float res = {};
      for (size_t i = 0; i < sizeof(data); i++) {
        res += (data[i] & 0x0f) * mul;
        mul *= 10.f;
        res += (data[i] >> 4) * mul;
        mul *= 10.f;
      }
      return res;
    }
  } PACKED;
  uint8_t header;               // FE
  uint8_t version;              // version? always 01
  uint8_t fields_count;         // fields count? always 08
  field_t<100> energy;          // kWh, divided by 100
  field_t<10> voltage;          // V, divided by 10
  field_t<10000> current;       // A, divided by 10000
  field_t<100> frequency;       // Hz, divided by 100
  field_t<100> active_power;    // W, divided by 100
  field_t<100> reactive_power;  // divided by 100
  field_t<100> apparent_power;  // divided by 100
  field_t<10> power_factor;     // %, divided by 10
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

  uint8_t calc_crc_(const void *data, size_t size);
  uint8_t check_crc_(const zmai90_data_t &data) { return this->calc_crc_(data, sizeof(data) - 1) == data.checksum; }

  void write_state(bool state) override;
};

}  // namespace zmai90v1
}  // namespace esphome
