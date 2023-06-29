#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_toothbrush {

class MiotToothbrush : public miot::MiotComponent, public sensor::Sensor {
 public:
  void dump_config() override;

  uint16_t get_product_id() const override { return this->product_id_; }
  const char *get_product_code() const override { return "Universal Toothbrush"; }
  bool process_mibeacon(const miot::MiBeacon &mib) override {
    if (this->product_id_ == 0) {
      this->product_id_ = mib.product_id;
    }
    return MiotComponent::process_mibeacon(mib);
  }

  void set_consumable_level(sensor::Sensor *consumable_level) { this->consumable_level_ = consumable_level; }

 protected:
  uint16_t product_id_ = {};
  sensor::Sensor *consumable_level_{nullptr};

  bool process_object_(const miot::BLEObject &obj) override;

  void process_toothbrush_event_(const miot::BLEObject &obj);
  void process_consumable_(const miot::BLEObject &obj);
};

}  // namespace miot_toothbrush
}  // namespace esphome
