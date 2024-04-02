#pragma once

#include <vector>
#include <functional>

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"

#include "qingping_beacon.h"

namespace esphome {
namespace qingping {

namespace espbt = esphome::esp32_ble_tracker;

class Qingping : public espbt::ESPBTDeviceListener, public Component {
 public:
  float get_setup_priority() const override { return setup_priority::AFTER_BLUETOOTH; }

  void dump_config() override;
  bool parse_device(const espbt::ESPBTDevice &device) override;

  void set_address(const uint64_t &address) { this->address_ = address; }
  void set_verbose(bool verbose) { this->verbose_ = verbose; }
  void set_use_beacon_mac(bool use_beacon_mac) { this->use_beacon_mac_ = use_beacon_mac; }
  void set_mac_exclude(std::set<uint64_t> &&mac_exclude) { this->mac_exclude_ = std::move(mac_exclude); }

  void add_on_state_callback(std::function<bool(const QPDataPoint &dp)> &&callback) {
    this->callbacks_.push_back(std::move(callback));
  }

  const uint8_t *get_mac() const {
    return this->address_ == 0 ? nullptr : reinterpret_cast<const uint8_t *>(&this->address_);
  }
  const uint64_t &get_address() const { return this->address_; }

 protected:
  uint64_t address_{};
  bool use_beacon_mac_{};
  bool verbose_{};
  std::set<uint64_t> mac_exclude_;
  std::vector<std::function<bool(const QPDataPoint &dp)>> callbacks_;
  bool process_beacon_(const uint64_t &address, const std::vector<uint8_t> &data);
  bool process_dp_(const uint8_t *data, size_t size);
};

class QingpingComponent : public Component {
 public:
  explicit QingpingComponent(Qingping *qp) : qp_(qp) {}

  float get_setup_priority() const override { return setup_priority::AFTER_BLUETOOTH; }

 protected:
  qingping::Qingping *qp_;
  void dump_config_(const char *tag);
};

}  // namespace qingping
}  // namespace esphome
