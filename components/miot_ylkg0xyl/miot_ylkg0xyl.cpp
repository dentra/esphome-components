#include "esphome/core/log.h"
#include "miot_ylkg0xyl.h"

namespace esphome {
namespace miot_ylkg0xyl {

static const char *const TAG = "miot_ylkg0xyl";

void MiotYLKG0XYL::dump_config() { this->dump_config_(TAG); }

bool MiotYLKG0XYL::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_BUTTON_EVENT:
      // processed by automation
      return false;
    default:
      return this->process_unhandled_(obj);
  }
  return false;
}

}  // namespace miot_ylkg0xyl
}  // namespace esphome
