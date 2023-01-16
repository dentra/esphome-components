#include "esphome/core/log.h"
#include "miot_th.h"

namespace esphome {
namespace miot_th {

static const char *const TAG = "miot_th";

constexpr uint16_t PRODUCT_ID_XMWSDJ04MMC = 0x1203;
constexpr uint16_t PRODUCT_ID_LYWSD02MMC = 0x16E4;

void MiotTH::dump_config() {
  this->dump_config_(TAG);
  LOG_SENSOR("  ", "Temperature", this);
  LOG_SENSOR("  ", "Humidity", this->humidity_);
}

void MiotTH::process_temperature_(const miot::BLEObject &obj) {
  const auto temperature = obj.get_temperature();
  if (temperature.has_value()) {
    this->publish_state(*temperature);
  }
}

void MiotTH::process_humidity_(const miot::BLEObject &obj) {
  if (this->humidity_ != nullptr) {
    const auto humidity = obj.get_humidity();
    if (humidity.has_value()) {
      this->humidity_->publish_state(*humidity);
    }
  }
}

void MiotTH::process_temperature_humidity_(const miot::BLEObject &obj) {
  const auto temperature_humidity = obj.get_temperature_humidity();
  if (temperature_humidity != nullptr) {
    this->publish_state(temperature_humidity->get_temperature());
    if (this->humidity_ != nullptr) {
      this->humidity_->publish_state(temperature_humidity->get_humidity());
    }
  }
}

void MiotTH::process_miaomiaoce_temperature_1001_(const miot::BLEObject &obj) {
  const auto temperature = obj.get_miaomiaoce_temperature_1001();
  if (temperature.has_value()) {
    this->publish_state(*temperature);
  }
}

void MiotTH::process_miaomiaoce_humidity_1008_(const miot::BLEObject &obj) {
  if (this->humidity_ != nullptr) {
    const auto humidity = obj.get_miaomiaoce_humidity_1008();
    if (humidity.has_value()) {
      this->humidity_->publish_state(*humidity);
    }
  }
}

void MiotTH::process_miaomiaoce_humidity_1002_(const miot::BLEObject &obj) {
  if (this->humidity_ != nullptr) {
    const auto humidity = obj.get_miaomiaoce_humidity_1002();
    if (humidity.has_value()) {
      this->humidity_->publish_state(*humidity);
    }
  }
}

bool MiotTH::process_mibeacon(const miot::MiBeacon &mib) {
  if (this->product_id_ == 0) {
    this->product_id_ = mib.product_id;
  }
  return MiotComponent::process_mibeacon(mib);
}

bool MiotTH::process_object_(const miot::BLEObject &obj) {
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

    case miot::MIID_MIAOMIAOCE_TEMPERATURE_1001:
      this->process_miaomiaoce_temperature_1001_(obj);
      break;

    case miot::MIID_MIAOMIAOCE_HUMIDITY_1008:
      this->process_miaomiaoce_humidity_1008_(obj);
      break;

    case miot::MIID_MIAOMIAOCE_HUMIDITY_1002:
      this->process_miaomiaoce_humidity_1002_(obj);
      break;

    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace miot_th
}  // namespace esphome
