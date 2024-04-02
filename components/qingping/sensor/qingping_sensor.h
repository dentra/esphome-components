#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

#include "../qingping.h"

namespace esphome {
namespace qingping {

template<class T> class QingpingSensor : public sensor::Sensor, public QingpingComponent {
  constexpr static const auto *TAG = "qingping_sensor";

 public:
  explicit QingpingSensor(Qingping *qp) : QingpingComponent(qp) {}

  void dump_config() override {
    LOG_SENSOR("", "Qingping Sensor", this);
    this->dump_config_(TAG);
  }

  void setup() override {
    if (this->qp_->get_mac() == nullptr) {
      this->mark_failed();
      this->set_internal(true);
      return;
    }
    this->qp_->add_on_state_callback([this](const QPDataPoint &dp) {
      auto pc = property_controller::make_state<T>(dp);
      if (!pc.valid()) {
        return false;
      }
      auto st = pc.state();
      if (std::isnan(st)) {
        return false;
      }
      if (this->get_force_update() || st != this->state) {
        this->publish_state(st);
      }
      return true;
    });
  }
};

}  // namespace qingping
}  // namespace esphome
