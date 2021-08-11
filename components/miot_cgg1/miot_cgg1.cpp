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

struct TemperatureHumidity {
  uint16_t temperature;
  uint16_t humidity;
};

void MiotCGG1::process_object_(const miot::BLEObject &obj) {
  auto temperature = obj.get_uint16(miot::ATTR_TEMPERATURE);
  if (temperature.has_value()) {
    const auto t = *temperature * 0.1f;
    ESP_LOGD(TAG, "  Temperature %.1f°C", t);
    this->publish_state(t);
  }
  auto humidity = obj.get_uint16(miot::ATTR_HUMIDITY);
  if (humidity.has_value()) {
    const auto h = *humidity * 0.1f;
    ESP_LOGD(TAG, " Humidity %.1f%%", h);
    if (this->humidity_ != nullptr) {
      this->humidity_->publish_state(h);
    }
  }
  auto both = obj.get_typed<TemperatureHumidity>(miot::ATTR_TEMPERATURE_HUMIDITY);
  if (both.has_value()) {
    const auto t = (*both)->temperature * 0.1f;
    const auto h = (*both)->humidity * 0.1f;
    ESP_LOGD(TAG, "  Temperature %.1f°C", t);
    ESP_LOGD(TAG, "  Humidity %.1f%%", h);
    this->publish_state(t);
    if (this->humidity_ != nullptr) {
      this->humidity_->publish_state(h);
    }
  }
}

}  // namespace miot_cgg1
}  // namespace esphome

#endif
