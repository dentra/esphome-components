#include "esphome/core/log.h"
#include "miot_ylyk01yl.h"

namespace esphome {
namespace miot_ylyk01yl {

static const char *const TAG = "miot_ylyk01yl";

void MiotYLYK01YL::dump_config() { this->dump_config_(TAG, "YLYK01YL"); }

bool MiotYLYK01YL::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_BUTTON_EVENT:
      // processed by automation
      return false;
    case miot::MIID_PAIRING_EVENT:
      // skip pairing event
      return false;
    default:
      return this->process_unhandled_(obj);
  }
  return false;
}

}  // namespace miot_ylyk01yl
}  // namespace esphome
