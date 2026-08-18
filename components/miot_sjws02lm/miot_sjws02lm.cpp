#include "esphome/core/log.h"
#include "miot_sjws02lm.h"

namespace esphome::miot_sjws02lm {

static const char *const TAG = "miot_sjws02lm";

void MiotSJWS02LM::dump_config() {
  this->dump_config_(TAG, "SJWS02LM");
  LOG_BINARY_SENSOR("  ", "Flooding", this);
}

void MiotSJWS02LM::process_flooding_(const miot::BLEObject &obj) {
  // This calls our updated get_flooding() method from miot_object.cpp
  const auto flooding = obj.get_flooding();
  if (flooding.has_value()) {
    this->publish_state(*flooding);
  }
}

bool MiotSJWS02LM::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_XIAOMI_FLOOD_2_BOTTOM:
    case miot::MIID_XIAOMI_FLOOD_2_TOP_PROP:
    case miot::MIID_XIAOMI_FLOOD_2_TOP_EVENT:
      this->process_flooding_(obj);
      break;

    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace esphome::miot_sjws02lm
