#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/log.h"
#include "miot_ylai003.h"

namespace esphome {
namespace miot_ylai003 {

static const char *const TAG = "miot_ylai003";

void MiotYLAI003::dump_config() { this->dump_config_(TAG); }

bool MiotYLAI003::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_BUTTON_EVENT:
      // processed by automation
      return false;
    case miot::MIID_PAIRING_EVENT:
      // skip pairing event
      return false;
    default:
      return this->process_default_(obj);
  }
  return false;
}

}  // namespace miot_ylai003
}  // namespace esphome

#endif
