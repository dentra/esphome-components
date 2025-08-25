#pragma once

#include "esphome/core/component.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_ylkg0xyl {

#define PRODUCT_ID_YLKG0xYL 0x03B6
#define PRODUCT_ID_MJGDYK01YL 0x15CE

class MiotYLKG0XYL : public miot::MiotComponent {
 public:
  MiotYLKG0XYL() { this->product_id_ = PRODUCT_ID_YLKG0xYL; }
  MiotYLKG0XYL(uint16_t product_id) { this->product_id_ = product_id; }

  void dump_config() override;

 protected:
  bool process_object_(const miot::BLEObject &obj) override;
};

}  // namespace miot_ylkg0xyl
}  // namespace esphome
