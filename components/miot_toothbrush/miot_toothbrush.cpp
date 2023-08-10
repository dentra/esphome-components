#include "esphome/core/log.h"
#include "miot_toothbrush.h"

namespace esphome {
namespace miot_toothbrush {

static const char *const TAG = "miot_toothbrush";

void MiotToothbrush::dump_config() {
  this->dump_config_(TAG, "Toothbrush");
  LOG_BINARY_SENSOR("", "Brush occupancy", this);
  LOG_SENSOR("  ", "Score", this->score_);
  LOG_SENSOR("  ", "Consumable Level", this->consumable_level_);
  LOG_SENSOR("  ", "Brushing Time", this->brushing_time_);
}

void MiotToothbrush::process_toothbrush_event_(const miot::BLEObject &obj) {
  const auto toothbrush_event = obj.get_toothbrush_event();
  if (toothbrush_event.has_value()) {
    if (toothbrush_event->type == miot::ToothbrushEvent::BRUSHING_START) {
      this->publish_state(true);
      this->brushing_start_ = toothbrush_event->timestamp;
    } else {
      this->publish_state(false);
      if (this->score_) {
        this->score_->publish_state(toothbrush_event->score);
      }
      if (this->brushing_time_ && this->brushing_start_) {
        this->brushing_time_->publish_state(toothbrush_event->timestamp - this->brushing_start_);
      }
      this->brushing_start_ = 0;
    }
  }
}

void MiotToothbrush::process_consumable_(const miot::BLEObject &obj) {
  if (this->consumable_level_) {
    const auto consumable = obj.get_consumable();
    if (consumable.has_value()) {
      this->consumable_level_->publish_state(*consumable);
    }
  }
}

bool MiotToothbrush::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_CONSUMABLE:
      this->process_consumable_(obj);
      break;
    case miot::MIID_XIAOBEI_TOOTHBRUSH_EVENT:
      this->process_toothbrush_event_(obj);
      break;
    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace miot_toothbrush
}  // namespace esphome
