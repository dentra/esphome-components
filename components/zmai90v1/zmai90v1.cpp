#include "esphome/core/log.h"
#include "zmai90v1.h"

namespace esphome {
namespace zmai90v1 {

static const char *const TAG = "zmai90v1";

static const uint8_t DATA_REQUEST[] = {0xfe, 0x01, 0x0f, 0x08, 0x00, 0x00, 0x00, 0x1c};

void ZMAi90v1::dump_config() {
  ESP_LOGCONFIG(TAG, "ZMAi-90 (V9821):");
  LOG_SWITCH("  ", "Switch", this);
  LOG_PIN("    Pin: ", this->switch_pin_);
  LOG_BINARY_SENSOR("  ", "Button", this->button_);
  LOG_PIN("    Pin: ", this->button_pin_);
  LOG_SENSOR("  ", "Energy", this->energy_);
  LOG_SENSOR("  ", "Voltage", this->voltage_);
  LOG_SENSOR("  ", "Current", this->current_);
  LOG_SENSOR("  ", "Frequency", this->frequency_);
  LOG_SENSOR("  ", "Active Power", this->active_power_);
  LOG_SENSOR("  ", "Reactive Power", this->reactive_power_);
  LOG_SENSOR("  ", "Apparent Power", this->apparent_power_);
  LOG_SENSOR("  ", "Power Factor", this->power_factor_);
}

void ZMAi90v1::setup() {
  auto switch_state = this->get_initial_state();
  if (switch_state.has_value()) {
    write_state(*switch_state);
  }
}

void ZMAi90v1::loop() {
  if (this->button_) {
    this->button_->publish_state(this->button_pin_->digital_read());
  }

  zmai90_data_t data = {};

  while (this->available() >= sizeof(zmai90_data_t)) {
    if (!this->read_array(reinterpret_cast<uint8_t *>(&data), sizeof(zmai90_data_t))) {
      ESP_LOGW(TAG, "Error read data buffer");
      break;
    }
    ESP_LOGD(TAG, "Got data: %s", hexencode(reinterpret_cast<uint8_t *>(&data), sizeof(zmai90_data_t)).c_str());
    if (data.header[0] != 0xFE && data.header[1] != 0x01 && data.header[2] != 0x08) {
      ESP_LOGW(TAG, "Invalid data header: %02X %02X %02X", data.header[0], data.header[1], data.header[2]);
      break;
    }
    if (this->calc_crc_(data) != data.checksum) {
      ESP_LOGW(TAG, "Invalid checksum");
      break;
    }
    if (this->energy_ != nullptr) {
      ESP_LOGD(TAG, "Reported energy: %s", hexencode(data.energy, sizeof(data.energy)).c_str());
      this->energy_->publish_state(this->get_val(data.energy, 0.01f));
    }
    if (this->voltage_ != nullptr) {
      ESP_LOGD(TAG, "Reported voltage: %s", hexencode(data.voltage, sizeof(data.voltage)).c_str());
      this->voltage_->publish_state(this->get_val(data.voltage, 0.1f));
    }
    if (this->current_ != nullptr) {
      ESP_LOGD(TAG, "Reported current: %s", hexencode(data.current, sizeof(data.current)).c_str());
      this->current_->publish_state(this->get_val(data.current, 0.0001f));
    }
    if (this->frequency_ != nullptr) {
      ESP_LOGD(TAG, "Reported frequency: %s", hexencode(data.frequency, sizeof(data.frequency)).c_str());
      this->frequency_->publish_state(this->get_val(data.frequency, 0.01f));
    }
    if (this->active_power_ != nullptr) {
      ESP_LOGD(TAG, "Reported active power: %s", hexencode(data.active_power, sizeof(data.active_power)).c_str());
      this->active_power_->publish_state(this->get_val(data.active_power, 0.01f));
    }
    if (this->reactive_power_ != nullptr) {
      ESP_LOGD(TAG, "Reported reactive power: %s", hexencode(data.reactive_power, sizeof(data.reactive_power)).c_str());
      this->reactive_power_->publish_state(this->get_val(data.reactive_power, 0.01f));
    }
    if (this->apparent_power_ != nullptr) {
      ESP_LOGD(TAG, "Reported apparent power: %s", hexencode(data.apparent_power, sizeof(data.apparent_power)).c_str());
      this->apparent_power_->publish_state(this->get_val(data.apparent_power, 0.01f));
    }
    if (this->power_factor_ != nullptr) {
      ESP_LOGD(TAG, "Reported power factor: %s", hexencode(data.power_factor, sizeof(data.power_factor)).c_str());
      this->power_factor_->publish_state(this->get_val(data.power_factor, 0.1f));
    }
  }
}

void ZMAi90v1::update() {
  ESP_LOGD(TAG, "Sending request: %s", hexencode(DATA_REQUEST, sizeof(DATA_REQUEST)).c_str());
  this->write_array(DATA_REQUEST, sizeof(DATA_REQUEST));
}

float ZMAi90v1::get_val(const uint8_t data[4], float mul) {
  float res = {};
  for (size_t i = 0; i < 4; i++) {
    res += (data[i] & 0x0f) * mul;
    mul *= 10.f;
    res += (data[i] >> 4) * mul;
    mul *= 10.f;
  }
  return res;
}

uint8_t ZMAi90v1::calc_crc_(const zmai90_data_t &data) {
  const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&data);
  uint8_t crc = 0;
  for (int i = 0; i < sizeof(data) - 1; i++) {
    crc += bytes[i];
  }
  return ~crc + 0x33;
}

void ZMAi90v1::write_state(bool state) {
  this->switch_pin_->digital_write(state);
  this->publish_state(state);
}

}  // namespace zmai90v1
}  // namespace esphome
