#pragma once
#ifdef ARDUINO_ARCH_ESP32
#include "esphome/core/component.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_ylkg0xyl {

constexpr uint16_t PRODUCT_ID = 0x03B6;

class MiotYLKG0XYL : public miot::MiotComponent {
 public:
  uint16_t get_product_id() const override { return PRODUCT_ID; }
  const char *get_product_code() const override { return "YLKG07YL/YLKG08YL"; }

  void dump_config() override;

 protected:
  bool process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_ylkg0xyl
}  // namespace esphome

#endif
