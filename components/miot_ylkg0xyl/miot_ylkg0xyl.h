#pragma once

#include "esphome/core/component.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_ylkg0xyl {

#define PRODUCT_ID_YLKG0xYL 0x03B6

class MiotYLKG0XYL : public miot::MiotComponent {
 public:
  MiotYLKG0XYL() { this->product_id_ = PRODUCT_ID_YLKG0xYL; }

  void dump_config() override;

 protected:
  bool process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_ylkg0xyl
}  // namespace esphome
