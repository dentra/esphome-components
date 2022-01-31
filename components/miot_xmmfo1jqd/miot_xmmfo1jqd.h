#pragma once

#include "esphome/core/component.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_xmmfo1jqd {

#define PRODUCT_ID_XMMFO1JQD 0x04E1

class MiotXMMFO1JQD : public miot::MiotComponent {
 public:
  uint16_t get_product_id() const override { return PRODUCT_ID_XMMFO1JQD; }
  const char *get_product_code() const override { return "XMMFO1JQD"; }

  void dump_config() override;

 protected:
  bool process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_xmmfo1jqd
}  // namespace esphome
