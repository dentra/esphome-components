#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/log.h"
#include "miot_ylyk01yl.h"

namespace esphome {
namespace miot_ylyk01yl {

static const char *TAG = "miot_ylyk01yl";

void MiotYLYK01YL::dump_config() { this->dump_config_(TAG); }

void MiotYLYK01YL::process_object_(const miot::BLEObject &obj) { this->process_default_(obj); }

}  // namespace miot_ylyk01yl
}  // namespace esphome

#endif
