#include "esphome/core/log.h"
#include "miot_thermogigro.h"

namespace esphome {
namespace miot_thermogigro {

static const char *const TAG = "miot_thermogigro";

constexpr uint16_t PRODUCT_ID_XMWSDJ04MMC = 0x1203;
constexpr uint16_t PRODUCT_ID_LYWSD02MMC = 0x16E4;

void MiotThermoGigro::dump_config() {
  this->dump_config_(TAG);
  LOG_SENSOR("  ", "Temperature", this);
  LOG_SENSOR("  ", "Humidity", this->humidity_);
}

void MiotThermoGigro::process_temperature_(const miot::BLEObject &obj) {
  const auto temperature = obj.get_temperature();
  if (temperature.has_value()) {
    this->publish_state(*temperature);
  }
}

void MiotThermoGigro::process_humidity_(const miot::BLEObject &obj) {
  if (this->humidity_ != nullptr) {
    const auto humidity = obj.get_humidity();
    if (humidity.has_value()) {
      this->humidity_->publish_state(*humidity);
    }
  }
}

void MiotThermoGigro::process_temperature_humidity_(const miot::BLEObject &obj) {
  const auto temperature_humidity = obj.get_temperature_humidity();
  if (temperature_humidity != nullptr) {
    this->publish_state(temperature_humidity->get_temperature());
    if (this->humidity_ != nullptr) {
      this->humidity_->publish_state(temperature_humidity->get_humidity());
    }
  }
}

void MiotThermoGigro::process_miaomiaoce_temperature_(const miot::BLEObject &obj) {
  // if (this->product_id_ != PRODUCT_ID_XMWSDJ04MMC) {
  //   return;
  // }
  const auto temperature = obj.get_miaomiaoce_temperature();
  if (temperature.has_value()) {
    this->publish_state(*temperature);
  }
}

void MiotThermoGigro::process_miaomiaoce_humidity_(const miot::BLEObject &obj) {
  // if (this->product_id_ != PRODUCT_ID_XMWSDJ04MMC) {
  //   return;
  // }
  if (this->humidity_ != nullptr) {
    const auto humidity = obj.get_miaomiaoce_humidity();
    if (humidity.has_value()) {
      this->humidity_->publish_state(*humidity);
    }
  }
}

void MiotThermoGigro::process_miaomiaoce_humidity_o2_(const miot::BLEObject &obj) {
  // if (this->product_id_ != PRODUCT_ID_LYWSD02MMC) {
  //   return;
  // }
  if (this->humidity_ != nullptr) {
    const auto humidity = obj.get_miaomiaoce_sensor_ht_o2_humidity();
    if (humidity.has_value()) {
      this->humidity_->publish_state(*humidity);
    }
  }
}

bool MiotThermoGigro::process_mibeacon(const miot::MiBeacon &mib) {
  if (this->product_id_ == 0) {
    this->product_id_ = mib.product_id;
  }
  return MiotComponent::process_mibeacon(mib);
}

bool MiotThermoGigro::process_object_(const miot::BLEObject &obj) {
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

    case miot::MIID_MIAOMIAOCE_TEMPERATURE:
      this->process_miaomiaoce_temperature_(obj);
      break;

    case miot::MIID_MIAOMIAOCE_HUMIDITY:
      this->process_miaomiaoce_humidity_(obj);
      break;

    case miot::MIID_MIAOMIAOCE_SENSOR_HT_O2_HUMIDITY:
      this->process_miaomiaoce_humidity_o2_(obj);
      break;

    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace miot_thermogigro
}  // namespace esphome
