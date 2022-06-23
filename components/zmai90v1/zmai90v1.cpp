#include "esphome/core/log.h"
#include "zmai90v1.h"

namespace esphome {
namespace zmai90v1 {

static const char *const TAG = "zmai90v1";

enum {
  DATA_HEADER = 0xFE,
  DATA_VERSION = 1,
  DATA_FIELDS_COUNT = 8,
};

static const uint8_t DATA_REQUEST[] = {
    DATA_HEADER,   // header
    DATA_VERSION,  // version?
    0x0F,          // unknown
    0x08,          // unknown
    0x00,          // unknown
    0x00,          // unknown
    0x00,          // unknown
    0x1C           // CRC
};

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
  bool initial_state = true;
  switch (this->restore_mode_) {
    case RESTORE_MODE_ALWAYS_ON:
      initial_state = true;
      break;
    case RESTORE_MODE_ALWAYS_OFF:
      initial_state = false;
      break;
    case RESTORE_MODE_RESTORE_DEFAULT_ON:
      initial_state = this->get_initial_state().value_or(true);
      break;
    case RESTORE_MODE_RESTORE_DEFAULT_OFF:
      initial_state = this->get_initial_state().value_or(false);
      break;
  }

  if (initial_state) {
    this->turn_on();
  } else {
    this->turn_off();
  }
  this->switch_pin_->setup();
  if (initial_state) {
    this->turn_on();
  } else {
    this->turn_off();
  }

  if (this->button_ != nullptr) {
    this->button_pin_->setup();
    this->button_->publish_initial_state(!this->button_pin_->digital_read());
  }
}

void ZMAi90v1::loop() {
  if (this->button_ != nullptr) {
    this->button_->publish_state(!this->button_pin_->digital_read());
  }

  zmai90_data_t data = {};

  while (this->available() >= sizeof(zmai90_data_t)) {
    if (!this->read_array(reinterpret_cast<uint8_t *>(&data), sizeof(zmai90_data_t))) {
      ESP_LOGW(TAG, "Error read data buffer");
      break;
    }
    ESP_LOGD(TAG, "Got data: %s", format_hex_pretty(reinterpret_cast<uint8_t *>(&data), sizeof(zmai90_data_t)).c_str());
    if (data.header != DATA_HEADER) {
      ESP_LOGW(TAG, "Invalid data header: %02X", data.header);
      break;
    }
    if (data.version != DATA_VERSION) {
      ESP_LOGW(TAG, "Invalid data header: %02X", data.version);
      break;
    }
    if (data.fields_count != DATA_FIELDS_COUNT) {
      ESP_LOGW(TAG, "Invalid data header: %u", data.fields_count);
      break;
    }
    if (!this->check_crc_(data)) {
      ESP_LOGW(TAG, "Invalid checksum");
      break;
    }
    if (this->energy_) {
      ESP_LOGD(TAG, "Reported energy: %s", data.energy.format_hex_pretty().c_str());
      this->energy_->publish_state(data.energy.value());
    }
    if (this->voltage_) {
      ESP_LOGD(TAG, "Reported voltage: %s", data.voltage.format_hex_pretty().c_str());
      this->voltage_->publish_state(data.voltage.value());
    }
    if (this->current_) {
      ESP_LOGD(TAG, "Reported current: %s", data.current.format_hex_pretty().c_str());
      this->current_->publish_state(data.current.value());
    }
    if (this->frequency_) {
      ESP_LOGD(TAG, "Reported frequency: %s", data.frequency.format_hex_pretty().c_str());
      this->frequency_->publish_state(data.frequency.value());
    }
    if (this->active_power_) {
      ESP_LOGD(TAG, "Reported active power: %s", data.active_power.format_hex_pretty().c_str());
      this->active_power_->publish_state(data.active_power.value());
    }
    if (this->reactive_power_) {
      ESP_LOGD(TAG, "Reported reactive power: %s", data.reactive_power.format_hex_pretty().c_str());
      this->reactive_power_->publish_state(data.reactive_power.value());
    }
    if (this->apparent_power_) {
      ESP_LOGD(TAG, "Reported apparent power: %s", data.apparent_power.format_hex_pretty().c_str());
      this->apparent_power_->publish_state(data.apparent_power.value());
    }
    if (this->power_factor_) {
      ESP_LOGD(TAG, "Reported power factor: %s", data.power_factor.format_hex_pretty().c_str());
      this->power_factor_->publish_state(data.power_factor.value());
    }
  }
}

void ZMAi90v1::update() {
  ESP_LOGD(TAG, "Sending request: %s", format_hex_pretty(DATA_REQUEST, sizeof(DATA_REQUEST)).c_str());
  this->write_array(DATA_REQUEST, sizeof(DATA_REQUEST));
}

uint8_t ZMAi90v1::calc_crc_(const void *data, size_t size) {
  auto data8 = static_cast<const uint8_t *>(data);
  uint8_t crc = 0;
  while (size--) {
    crc += *data8++;
  }
  return ~crc + 0x33;
}

void ZMAi90v1::write_state(bool state) {
  this->switch_pin_->digital_write(state);
  this->publish_state(state);
}

}  // namespace zmai90v1
}  // namespace esphome
