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

void MiotSJWS01LM::process_object_(const miot::BLEObject &obj) {
  auto flooding = obj.get_bool(miot::ATTR_FLOODING);
  if (flooding.has_value()) {
    ESP_LOGD(TAG, "  Flooding: %s", YESNO(*flooding));
    this->publish_state(*flooding);
  }
}

}  // namespace miot_sjws01lm
}  // namespace esphome

#endif
