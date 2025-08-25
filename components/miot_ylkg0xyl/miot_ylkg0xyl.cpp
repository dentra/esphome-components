#include "esphome/core/log.h"
#include "miot_ylkg0xyl.h"

namespace esphome {
namespace miot_ylkg0xyl {

static const char *const TAG = "miot_ylkg0xyl";

void MiotYLKG0XYL::dump_config() {
  const char* device_name;
  switch (this->product_id_) {
    case PRODUCT_ID_MJGDYK01YL:
      device_name = "MJGDYK01YL";
      break;
    case PRODUCT_ID_YLKG0xYL:
    default:
      device_name = "YLKG07YL/YLKG08YL";
      break;
  }
  this->dump_config_(TAG, device_name);
}

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
