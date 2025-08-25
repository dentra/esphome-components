#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include "../miot/miot.h"
#include "miot_ylkg0xyl.h"

namespace esphome {
namespace miot_ylkg0xyl {

enum MiotYLKG0XYLEvent {
  ON_SHORT_PRESS = 1,
  ON_LONG_PRESS,
  ON_ROTATE_LEFT,
  ON_ROTATE_RIGHT,
  ON_ROTATE_LEFT_PRESSED,
  ON_ROTATE_RIGHT_PRESSED,
};

class MiotYLKG0XYLTrigger : public Trigger<uint8_t>, public miot::MiotListener {
 public:
  explicit MiotYLKG0XYLTrigger(MiotYLKG0XYLEvent event) : event_(event) { this->product_id_ = PRODUCT_ID_YLKG0xYL; }
  MiotYLKG0XYLTrigger(MiotYLKG0XYLEvent event, uint16_t product_id) : event_(event) { this->product_id_ = product_id; }

 protected:
  MiotYLKG0XYLEvent event_;

  bool process_object_(const miot::BLEObject &obj) override;
  uint8_t process_button_event_(const miot::ButtonEvent &button_event);
};

}  // namespace miot_ylkg0xyl
}  // namespace esphome
