#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/version.h"

#include "esphome/components/switch/switch.h"

#if (ESPHOME_VERSION_CODE < VERSION_CODE(2024, 6, 0))
#include "esphome/components/ota/ota_component.h"
#else
#include "esphome/components/esphome/ota/ota_esphome.h"
#endif

namespace esphome {
namespace otax {

#if (ESPHOME_VERSION_CODE < VERSION_CODE(2024, 6, 0))
using ESPHomeOTAComponent = ota::OTAComponent;
#endif

class Otax : public ESPHomeOTAComponent {
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
