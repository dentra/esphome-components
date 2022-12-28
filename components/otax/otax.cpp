#include "esphome/core/log.h"

#include "otax.h"

namespace esphome {
namespace otax {

static const char *const TAG = "otax";

void Otax::dump_config() {
  ota::OTAComponent::dump_config();
  LOG_SWITCH(" ", "Enabled", this->enabled_);
}

void Otax::loop() {
  if (this->enabled_ == nullptr || this->enabled_->state) {
    ota::OTAComponent::loop();
  }
}

}  // namespace otax
}  // namespace esphome
