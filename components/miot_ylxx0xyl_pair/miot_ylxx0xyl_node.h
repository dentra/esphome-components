#pragma once

#include "esphome/components/ble_client/ble_client.h"
#include "../miot_client/miot_client.h"
#include "../miot_client/miot_legacy_bond_client.h"

namespace esphome {
namespace miot_ylxx0xyl {

class YLxx0xYLPublisher {
 public:
  virtual void publish_bindkey(const std::string &value) = 0;
  virtual void publish_version(const std::string &value) = 0;
};

class MiotYLxx0xYLNode : public miot_client::MiotClient, public miot_client::AuthClientListener {
 public:
  MiotYLxx0xYLNode(YLxx0xYLPublisher *publisher) : publisher_(publisher) {}

  void on_disconnect(const esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param &param) override;
  void on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) override;
  void on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) override;

  void on_auth_complete() override;

  void start_pairing();

 protected:
  struct {
    uint16_t ver;
    uint16_t key;
  } char_ = {};

  YLxx0xYLPublisher *publisher_;

  // void (MiotYLxx0xYLNode::*step)();
  // uint16_t step_handle_ = {};
  // void do_step_(uint16_t handle);

  void on_read_ver_(const uint8_t *data, int size);
  void on_read_key_(const uint8_t *data, int size);
};

}  // namespace miot_ylxx0xyl
}  // namespace esphome
