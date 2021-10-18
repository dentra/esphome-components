#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/log.h"
#include "miot_mhoc303.h"

namespace esphome {
namespace miot_mhoc303 {

static const char *const TAG = "miot_mhoc303";

void MiotMHOC303::dump_config() {
  this->dump_config_(TAG);
  LOG_SENSOR("  ", "Temperature", this);
  LOG_SENSOR("  ", "Humidity", this->humidity_);
}

void MiotMHOC303::process_temperature_(const miot::BLEObject &obj) {
  const auto temperature = obj.get_temperature();
  if (temperature.has_value()) {
    this->publish_state(*temperature);
  }
}

void MiotMHOC303::process_humidity_(const miot::BLEObject &obj) {
  if (this->humidity_ != nullptr) {
    const auto humidity = obj.get_humidity();
    if (humidity.has_value()) {
      this->humidity_->publish_state(*humidity);
    }
  }
}

bool MiotMHOC303::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_TEMPERATURE:
      this->process_temperature_(obj);
      break;

    case miot::MIID_HUMIDITY:
      this->process_humidity_(obj);
      break;

    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace miot_mhoc303
}  // namespace esphome

#endif
