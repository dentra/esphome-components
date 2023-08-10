#pragma once
#include "esphome/core/component.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_ylyk01yl {

#define PRODUCT_ID_YLYK01YL 0x0153

class MiotYLYK01YL : public miot::MiotComponent {
 public:
  MiotYLYK01YL() { this->product_id_ = PRODUCT_ID_YLYK01YL; }

  void dump_config() override;

 protected:
  bool process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_ylyk01yl
}  // namespace esphome
