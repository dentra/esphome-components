#pragma once
#ifdef ARDUINO_ARCH_ESP32
#include "esphome/core/component.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_ylyk01yl {

constexpr uint16_t PRODUCT_ID = 0x0153;

class MiotYLYK01YL : public miot::MiotComponent {
 public:
  uint16_t get_product_id() const override { return PRODUCT_ID; }
  const char *get_product_code() const override { return "YLYK01YL"; }

  void dump_config() override;

 protected:
  bool process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_ylyk01yl
}  // namespace esphome

#endif
