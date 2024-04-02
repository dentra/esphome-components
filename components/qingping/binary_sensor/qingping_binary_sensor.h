#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "../qingping.h"

#include "qingping_binary_sensor_pc.h"

namespace esphome {
namespace qingping {

template<class T> class QingpingBinarySensor : public binary_sensor::BinarySensor, public QingpingComponent {
  constexpr static const auto *TAG = "qingping_binary_sensor";

 public:
  explicit QingpingBinarySensor(Qingping *qp) : QingpingComponent(qp) {}

  void dump_config() override {
    LOG_BINARY_SENSOR("", "Qingping BinarySensor", this);
    this->dump_config_(TAG);
  }

  void setup() override {
    if (this->qp_->get_mac() == nullptr) {
      this->mark_failed();
      this->set_internal(true);
      return;
    }
    this->qp_->add_on_state_callback([this](const QPDataPoint &dp) {
      auto pc = property_controller::make_binary_state<T>(dp);
      if (!pc.valid()) {
        return false;
      }
      this->publish_state(pc.state());
      return true;
    });
  }
};

}  // namespace qingping
}  // namespace esphome
