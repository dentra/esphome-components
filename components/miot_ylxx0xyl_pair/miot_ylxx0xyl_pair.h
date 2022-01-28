#pragma once

#include "esphome/components/text_sensor/text_sensor.h"
#include "../miot/miot.h"
#include "miot_ylxx0xyl_node.h"

namespace esphome {
namespace miot_ylxx0xyl {

class MiotYLxx0xYLPair : public Component,
                         public miot::MiotListener,
                         public text_sensor::TextSensor,
                         public YLxx0xYLPublisher {
 public:
  float get_setup_priority() const override { return setup_priority::LATE; }
  void dump_config() override;

  uint16_t get_product_id() const override { return this->product_id_; }
  void set_product_id(uint16_t product_id) { this->product_id_ = product_id; }

  void set_auth_client(miot_client::MiotLegacyBondClient *auth) { this->auth_ = auth; }
  void set_version(text_sensor::TextSensor *version) { this->version_ = version; }

  void publish_bindkey(const std::string &value) override { this->publish_state(value); }
  void publish_version(const std::string &value) override {
    if (this->version_ != nullptr) {
      this->version_->publish_state(value);
    }
  }

  bool process_mibeacon(const miot::MiBeacon &mib) override;

 protected:
  text_sensor::TextSensor *version_ = {};
  uint16_t product_id_ = {};
  miot_client::MiotLegacyBondClient *auth_ = {};
  void process_pairing_(const miot::BLEObject &obj);
  bool process_object_(const miot::BLEObject &obj) override { return false; };
};

}  // namespace miot_ylxx0xyl
}  // namespace esphome
