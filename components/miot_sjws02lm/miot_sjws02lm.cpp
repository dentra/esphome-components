#include "esphome/core/log.h"
#include "miot_sjws02lm.h"

namespace esphome {
namespace miot_sjws02lm {

static const char *const TAG = "miot_sjws02lm";

void MiotSJWS02LM::dump_config() {
  this->dump_config_(TAG, "SJWS02LM");
  LOG_BINARY_SENSOR("  ", "Water Leak", this);
  LOG_BINARY_SENSOR("  ", "Bottom", this->bottom_);
  LOG_BINARY_SENSOR("  ", "Top", this->top_);
}

void MiotSJWS02LM::publish_bottom_(bool state) {
  this->bottom_state_ = state;
  if (this->bottom_ != nullptr) {
    this->bottom_->publish_state(state);
  }
  this->publish_water_leak_();
}

void MiotSJWS02LM::publish_top_(bool state) {
  this->top_state_ = state;
  if (this->top_ != nullptr) {
    this->top_->publish_state(state);
  }
  this->publish_water_leak_();
}

void MiotSJWS02LM::publish_water_leak_() {
  this->publish_state(this->bottom_state_.value_or(false) || this->top_state_.value_or(false));
}

void MiotSJWS02LM::process_water_leak_event_(const miot::BLEObject &obj) {
  const auto event_type = obj.get_water_leak_event();
  if (!event_type.has_value()) {
    return;
  }
  switch (*event_type) {
    // bottom wet
    case 0x00:
      this->publish_bottom_(true);
      break;
    // bottom dry
    case 0x01:
      this->publish_bottom_(false);
      break;
    // top wet
    case 0x02:
      this->publish_top_(true);
      break;
    // top dry
    case 0x03:
      this->publish_top_(false);
      break;
    default:
      ESP_LOGW(TAG, "Unknown water leak event: %" PRIu8, *event_type);
      break;
  }
}

bool MiotSJWS02LM::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_WATER_LEAK_BOTTOM: {
      const auto state = obj.get_water_leak_bottom();
      if (state.has_value()) {
        this->publish_bottom_(*state);
      }
      break;
    }

    case miot::MIID_WATER_LEAK_TOP: {
      const auto state = obj.get_water_leak_top();
      if (state.has_value()) {
        this->publish_top_(*state);
      }
      break;
    }

    case miot::MIID_WATER_LEAK_EVENT:
      this->process_water_leak_event_(obj);
      break;

    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace miot_sjws02lm
}  // namespace esphome
