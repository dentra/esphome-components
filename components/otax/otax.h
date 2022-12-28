#pragma once

#include "esphome/core/defines.h"
#include "esphome/components/ota/ota_component.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace otax {

class Otax : public ota::OTAComponent {
 public:
  void dump_config() override;
  void loop() override;

  void set_enabled(switch_::Switch *enabled) { this->enabled_ = enabled; }

  class Enabled : public switch_::Switch {
   protected:
    void write_state(bool state) override { this->publish_state(state); }
  };

 protected:
  switch_::Switch *enabled_{};
};

}  // namespace otax
}  // namespace esphome
