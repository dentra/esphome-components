#pragma once

#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/core/automation.h"
#include "miot.h"

namespace esphome {
namespace miot {

using namespace esp32_ble_tracker;

class MiotAdvertiseTrigger : public Trigger<const BLEObject &>, public MiotListener {
 public:
  void set_product_id(uint16_t product_id) { this->product_id_ = product_id; }
  void set_debug(bool debug) { this->debug_ = debug; }

  bool process_mibeacon(const MiBeacon &mib) override;

 protected:
  bool debug_{};

  bool process_object_(const BLEObject &obj) override {
    this->trigger(obj);
    return true;
  };
};

}  // namespace miot
}  // namespace esphome
