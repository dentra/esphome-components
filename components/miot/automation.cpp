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
  return MiotListener::process_mibeacon(mib);
}

}  // namespace miot
}  // namespace esphome
