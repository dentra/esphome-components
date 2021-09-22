#pragma once
#include "esphome/core/application.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/ble_client/ble_client.h"
#include "../miot/miot.h"
#include "miot_client_node.h"
#include "miot_client.h"

namespace esphome {
namespace miot_ylxx0xyl_pair {

class MiotYLxx0xYLPair;

// https://github.com/archaron/docs/blob/master/BLE/ylkg08y.md
// 1. Send 0x90, 0xCA, 0x85, 0xDE bytes to authInitCharacteristic.
// 2. Subscribe authCharacteristic.
// 3. Send cipher(mixA(reversedMac, productID), token) to authCharacteristic.
// 4. Now you'll get a notification on authCharacteristic. You must wait for this notification before proceeding to
// next step. The notification data can be ignored or used to check an integrity, this is optional. If you want to
// perform a check, compare cipher(mixB(reversedMac, productID), cipher(mixA(reversedMac, productID), res)) where
// res is received payload with your token, they must equal.
// 5. Send 0x92, 0xAB, 0x54, 0xFA to authCharacteristic.
// 6. Read from verCharacteristics. You can ignore the response data, you just have to perform a read to complete
// authentication process.
class MiotYLxx0xYLPairNode : public miot_client::MiotClientNode {
 public:
  MiotYLxx0xYLPairNode(MiotYLxx0xYLPair *pair) : pair_(pair) {}

  void on_disconnect(const esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param &param) override;
  void on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) override;
  void on_write_char(const esp_ble_gattc_cb_param_t::gattc_write_evt_param &param) override;
  void on_register_for_notification(const esp_ble_gattc_cb_param_t::gattc_reg_for_notify_evt_param &param) override;
  void on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) override;
  void on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) override;

  void start_pairing();

 protected:
  struct {
    // authInitCharacteristic
    uint16_t init;
    // authCharacteristic
    uint16_t auth;
    // verCharacteristics
    uint16_t ver;
    uint16_t key;
  } characteristics_ = {};
  std::vector<uint8_t> token_;
  enum { NONE = 0, STEP1 = 1, STEP2 = 2, STEP3 = 3, STEP4 = 4, STEP5 = 5, STEP6 = 6 } progress_ = NONE;
  MiotYLxx0xYLPair *pair_;

  // void (MiotYLxx0xYLPairNode::*step)();
  // uint16_t step_handle_ = {};
  // void do_step_(uint16_t handle);
  void step1_();
  void step2_();
  void step3_();
  void step5_();
  void step6_();
  void on_read_ver_(const uint8_t *data, int size);
  void on_read_key_(const uint8_t *data, int size);
};

class MiotYLxx0xYLPair : public miot::MiotComponent, public text_sensor::TextSensor {
 public:
  void dump_config() override;
  // void setup() override { this->client_.setup(); }
  // void loop() override { this->client_.loop(); }

  uint16_t get_product_id() const override { return product_id_; }
  const char *get_product_code() const override { return "PairYLxx0xYL"; }

  void set_ble_tracker(esp32_ble_tracker::ESP32BLETracker *ble_tracker) {
    this->client_.set_component_source("miot");
    App.register_component(&this->client_);
    ble_tracker->register_client(&this->client_);
    this->client_.set_address(this->get_address());
    auto node = new MiotYLxx0xYLPairNode(this);
    this->client_.register_ble_node(node);
  }

  void set_bindkey(text_sensor::TextSensor *bindkey) { this->bindkey_ = bindkey; }
  void set_version(text_sensor::TextSensor *version) { this->version_ = version; }

  void publish_bindkey(const std::string &value) {
    if (this->bindkey_ != nullptr) {
      this->bindkey_->publish_state(value);
    }
  }
  void publish_version(const std::string &value) {
    if (this->version_ != nullptr) {
      this->version_->publish_state(value);
    }
  }

  bool process_mibeacon(const miot::MiBeacon &mib) override;

 protected:
  text_sensor::TextSensor *bindkey_ = {};
  text_sensor::TextSensor *version_ = {};
  uint16_t product_id_ = {};
  miot_client::MiotClient client_;
  bool process_object_(const miot::BLEObject &obj) override;
  void process_pairing_(const miot::BLEObject &obj);
};

}  // namespace miot_ylxx0xyl_pair
}  // namespace esphome
