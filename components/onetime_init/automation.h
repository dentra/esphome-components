#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace onetime_init {

class OnetimeInitTrigger : public Trigger<>, public Component {
 public:
  explicit OnetimeInitTrigger(float setup_priority) : setup_priority_(setup_priority) {}
  void setup() override {
    ESPPreferenceObject pref = global_preferences->make_preference<uint8_t>(fnv1_hash("onetime_init"), true);
    uint8_t prev_state{};
    if (!pref.load(&prev_state) || prev_state == 0) {
      this->trigger();
      prev_state = 1;
      pref.save(&prev_state);
      global_preferences->sync();
    }
  }
  float get_setup_priority() const override { return this->setup_priority_; }

 protected:
  float setup_priority_;
};

}  // namespace onetime_init
}  // namespace esphome
