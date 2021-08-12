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
  auto temperature = obj.get_int16();
  if (temperature.has_value()) {
    const auto t = *temperature * 0.1f;
    ESP_LOGD(TAG, "Temperature %.1f°C", t);
    this->publish_state(t);
  }
}

void MiotCGG1::process_humidity_(const miot::BLEObject &obj) {
  if (this->humidity_ != nullptr) {
    auto humidity = obj.get_uint16();
    if (humidity.has_value()) {
      const auto h = *humidity * 0.1f;
      ESP_LOGD(TAG, "Humidity %.1f%%", h);
      this->humidity_->publish_state(h);
    }
  }
}
void MiotCGG1::process_temperature_humidity_(const miot::BLEObject &obj) {
  struct TemperatureHumidity {
    uint16_t temperature;
    uint16_t humidity;
  };
  auto both = obj.get_typed<TemperatureHumidity>();
  if (both.has_value()) {
    const auto t = (*both)->temperature * 0.1f;
    ESP_LOGD(TAG, "Temperature %.1f°C", t);
    this->publish_state(t);
    if (this->humidity_ != nullptr) {
      const auto h = (*both)->humidity * 0.1f;
      ESP_LOGD(TAG, "Humidity %.1f%%", h);
      this->humidity_->publish_state(h);
    }
  }
}

void MiotCGG1::process_object_(const miot::BLEObject &obj) {
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
      this->process_default_(obj);
      break;
  }
}

}  // namespace miot_cgg1
}  // namespace esphome

#endif
