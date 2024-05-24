#include "esphome/core/log.h"

#include "otax.h"

namespace esphome {
namespace otax {

static const char *const TAG = "otax";

void Otax::dump_config() {
  ESPHomeOTAComponent::dump_config();
  LOG_SWITCH(" ", "Enabled", this->enabled_);
}

void Otax::loop() {
  if (this->enabled_ == nullptr || this->enabled_->state) {
    ESPHomeOTAComponent::loop();
  }
}

}  // namespace otax
}  // namespace esphome
