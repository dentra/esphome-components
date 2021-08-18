#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/log.h"
#include "miot_sjws01lm.h"

namespace esphome {
namespace miot_sjws01lm {

static const char *TAG = "miot_sjws01lm";

void MiotSJWS01LM::dump_config() {
  this->dump_config_(TAG);
  LOG_BINARY_SENSOR("  ", "Flood Detector", this);
}

void MiotSJWS01LM::process_flooding_(const miot::BLEObject &obj) {
  const auto flooding = obj.get_flooding();
  if (flooding.has_value()) {
    this->publish_state(*flooding);
  }
}

void MiotSJWS01LM::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_FLOODING:
      this->process_flooding_(obj);
      break;

    default:
      this->process_default_(obj);
      break;
  }
}

}  // namespace miot_sjws01lm
}  // namespace esphome

#endif
