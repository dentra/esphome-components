#pragma once
#include <array>
#include "esphome/components/ble_client/ble_client.h"
#include "miot_client.h"
#include "miot_client_auth.h"

namespace esphome {
namespace miot_client {

/**
 * Provide auth throught BLE_UUID::MI_STANDARD_AUTH.
 *
 * 1. Read version
 * 2
 */
class MiotStandardAuthClient : public MiotClient, public AuthClient {
 public:
  void on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) override;
  void on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) override;
  void on_register_for_notification(const esp_ble_gattc_cb_param_t::gattc_reg_for_notify_evt_param &param) override;

  optional<std::vector<uint8_t>> decode(const uint8_t *data, const uint16_t size) const override;
  optional<std::vector<uint8_t>> encode(const uint8_t *data, const uint16_t size) override;
  bool is_auth_complete() const override { return is_established(); }

  void set_bindkey(const uint8_t bindkey[16]) { std::memcpy(this->beakonkey_, bindkey, sizeof(this->beakonkey_)); }
  void set_token(const uint8_t token[12]) { std::memcpy(this->token_, token, sizeof(this->token_)); }

 protected:
  struct {
    uint16_t version;
    uint16_t ctrlp;
    uint16_t auth;
  } char_ = {};

  AuthFrame::Cmd state_{AuthFrame::CMD_PASS_THROUGH};

  struct LoadCtx {
    uint8_t app_random_data[16] = {};
    uint8_t dev_random_data[16] = {};
    uint8_t app_confirmation_data[32] = {};
    uint8_t dev_confirmation_data[32] = {};
    uint8_t exp_confirmation_data[32] = {};
    uint16_t max_rx_frames = {};
  };

  LoadCtx *load_ctx_ = {};

  uint8_t token_[12] = {};
  uint8_t beakonkey_[16] = {};
  struct {
    uint8_t dev_key[16];
    uint8_t app_key[16];
    uint32_t dev_iv;
    uint32_t app_iv;
    uint8_t unknown[24];
  } PACKED session_ctx_ = {};

  uint16_t auth_seq_ = {};

  void process_ctrlp_(const OpCode opcode);
  void process_auth_(const AuthFrame &frame, uint16_t frame_size);
  void process_auth_app_random_(const AuthFrame &frame, uint16_t frame_size);
  void process_auth_dev_random_(const AuthFrame &frame, uint16_t frame_size);
  void process_auth_app_confirmation_(const AuthFrame &frame, uint16_t frame_size);
  void process_auth_dev_confirmation_(const AuthFrame &frame, uint16_t frame_size);
  void generate_login_data_();

  bool write_opcode_(OpCode opcode) { return this->write_char(this->char_.ctrlp, static_cast<uint32_t>(opcode)); }
  bool write_cmd_(AuthFrame::Cmd cmd, size_t data_size = 0);
  bool write_ack_(AuthFrame::Ack ack);
  bool write_data_(const uint8_t *data, const size_t size_size);
};

}  // namespace miot_client
}  // namespace esphome
