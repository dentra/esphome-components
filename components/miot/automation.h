#pragma once
#ifdef ARDUINO_ARCH_ESP32

#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/core/automation.h"
#include "miot.h"

namespace esphome {
namespace miot {

using namespace esp32_ble_tracker;

class MiotAdvertiseTrigger : public Trigger<const BLEObject &>, public MiotListener {
 public:
  explicit MiotAdvertiseTrigger(ESP32BLETracker *parent) { parent->register_listener(this); }

  void set_product_id(uint16_t product_id) { this->product_id_ = product_id; }
  uint16_t get_product_id() const override { return this->product_id_; }
  void set_debug(bool debug) { this->debug_ = debug; }

 protected:
  bool debug_ = {};
  uint16_t product_id_ = {};
  bool process_mibeacon_(const MiBeacon &mib) override;
  void process_object_(const BLEObject &obj) override { this->trigger(obj); };
};

}  // namespace miot
}  // namespace esphome
#endif
