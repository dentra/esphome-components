#include "esphome/core/log.h"
#include "miot_kettle.h"

namespace esphome {
namespace miot_kettle {

static const char *const TAG = "miot_kettle";

void MiotKettle::dump_config() {
  this->dump_config_(TAG, "Kettle");
  LOG_BINARY_SENSOR("  ", "Water Boil Power", this);
  LOG_SENSOR(" ", "Water Boil Temperature", this->temperature_);
}

void MiotKettle::process_water_boil_(const miot::BLEObject &obj) {
  const auto water_boil = obj.get_water_boil();
  if (water_boil != nullptr) {
    this->publish_state(water_boil->get_power());
    if (this->temperature_ != nullptr) {
      this->temperature_->publish_state(water_boil->get_temperature());
    }
  }
}

bool MiotKettle::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_WATER_BOIL:
      this->process_water_boil_(obj);
      break;

    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace miot_kettle
}  // namespace esphome
