#include "esphome/core/log.h"
#include "miot_xmmfo1jqd.h"

namespace esphome {
namespace miot_xmmfo1jqd {

static const char *const TAG = "miot_xmmfo1jqd";

void MiotXMMFO1JQD::dump_config() { this->dump_config_(TAG); }

bool MiotXMMFO1JQD::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_BUTTON_EVENT:
      // processed by automation
      return false;
    default:
      return this->process_unhandled_(obj);
  }
  return false;
}

}  // namespace miot_xmmfo1jqd
}  // namespace esphome
