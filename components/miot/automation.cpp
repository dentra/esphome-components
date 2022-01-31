#include "esphome/core/log.h"
#include "automation.h"
#include "miot_dump.h"

namespace esphome {
namespace miot {

static const char *const TAG = "miot.automation";

bool MiotAdvertiseTrigger::process_mibeacon(const MiBeacon &mib) {
  if (this->debug_) {
    dump(TAG, mib);
  }
  if (this->product_id_ != 0) {
    return MiotListener::process_mibeacon(mib);
  }
  this->product_id_ = mib.product_id;
  bool result = MiotListener::process_mibeacon(mib);
  this->product_id_ = 0;
  return result;
}

}  // namespace miot
}  // namespace esphome
