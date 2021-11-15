#pragma once
#ifdef ARDUINO_ARCH_ESP32
#include "esphome/core/component.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_ylyk01yl {

#define PRODUCT_ID_YLYK01YL 0x0153

class MiotYLYK01YL : public miot::MiotComponent {
 public:
  uint16_t get_product_id() const override { return PRODUCT_ID_YLYK01YL; }
  const char *get_product_code() const override { return "YLYK01YL"; }

  void dump_config() override;

 protected:
  bool process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_ylyk01yl
}  // namespace esphome

#endif
