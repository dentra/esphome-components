#pragma once
#ifdef ARDUINO_ARCH_ESP32
#include "esphome/core/component.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_ylai003 {

constexpr uint16_t PRODUCT_ID = 0x07BF;

class MiotYLAI003 : public miot::MiotComponent {
 public:
  uint16_t get_product_id() const override { return PRODUCT_ID; }
  const char *get_product_code() const override { return "YLAI003"; }

  void dump_config() override;

 protected:
  bool process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_ylai003
}  // namespace esphome

#endif
