#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/log.h"
#include "miot_cgg1.h"

namespace esphome {
namespace miot_cgg1 {

static const char *TAG = "miot_cgg1";

void MiotCGG1::dump_config() {
  this->dump_config_(TAG);
  LOG_SENSOR("  ", "Temperature", this);
  LOG_SENSOR("  ", "Humidity", this->humidity_);
}

void MiotCGG1::process_temperature_(const miot::BLEObject &obj) {
  const auto temperature = obj.get_temperature();
  if (temperature.has_value()) {
    this->publish_state(*temperature);
  }
}

void MiotCGG1::process_humidity_(const miot::BLEObject &obj) {
  if (this->humidity_ != nullptr) {
    const auto humidity = obj.get_humidity();
    if (humidity.has_value()) {
      this->humidity_->publish_state(*humidity);
    }
  }
}

void MiotCGG1::process_temperature_humidity_(const miot::BLEObject &obj) {
  const auto temperature_humidity = obj.get_temperature_humidity();
  if (temperature_humidity.has_value()) {
    this->publish_state(temperature_humidity->temperature);
    if (this->humidity_ != nullptr) {
      this->humidity_->publish_state(temperature_humidity->humidity);
    }
  }
}

bool MiotCGG1::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_TEMPERATURE:
      this->process_temperature_(obj);
      break;

    case miot::MIID_HUMIDITY:
      this->process_humidity_(obj);
      break;

    case miot::MIID_TEMPERATURE_HUMIDITY:
      this->process_temperature_humidity_(obj);
      break;

    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace miot_cgg1
}  // namespace esphome

#endif
