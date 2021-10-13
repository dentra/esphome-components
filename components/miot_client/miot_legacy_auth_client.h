#pragma once
#include "esphome/components/ble_client/ble_client.h"
#include "miot_client.h"

namespace esphome {
namespace miot_client {

/**
 * Provice auth throught BT_UUID_MI_LEGACY_AUTH.
 *
 * https://github.com/archaron/docs/blob/master/BLE/ylkg08y.md
 * 1. Send 0x90, 0xCA, 0x85, 0xDE bytes to authInitCharacteristic.
 * 2. Subscribe authCharacteristic.
 * 3. Send cipher(mixA(reversedMac, productID), token) to authCharacteristic.
 * 4. Now you'll get a notification on authCharacteristic. You must wait for
 *    this notification before proceeding to next step. The notification data
 *    can be ignored or used to check an integrity, this is optional. If you
 *    want to perform a check, compare cipher(mixB(reversedMac, productID),
 *    cipher(mixA(reversedMac, productID), res)) where res is received payload
 *    with your token, they must equal.
 * 5. Send 0x92, 0xAB, 0x54, 0xFA to authCharacteristic.
 * 6. Read from verCharacteristics. You can ignore the response data, you just
 *    have to perform a read to complete authentication process.
 */
class MiotLegacyAuthClient : public MiotClient, public AuthClient {
 public:
  void on_disconnect(const esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param &param) override;
  void on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) override;
  void on_write_char(const esp_ble_gattc_cb_param_t::gattc_write_evt_param &param) override;
  void on_register_for_notification(const esp_ble_gattc_cb_param_t::gattc_reg_for_notify_evt_param &param) override;
  void on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) override;
  void on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) override;

  void start_auth();

  void set_product_id(uint16_t product_id) { this->product_id_ = product_id; }

  std::vector<uint8_t> decode(const uint8_t *data, const uint16_t size) const override;
  std::vector<uint8_t> encode(const uint8_t *data, const uint16_t size) const override;
  bool is_auth_complete() const override;

 protected:
  uint16_t product_id_ = {};
  struct {
    uint16_t ctrlp;
    uint16_t auth;
    uint16_t ver;
  } char_ = {};
  std::vector<uint8_t> token_;
  enum { NONE = 0, STEP1 = 1, STEP2 = 2, STEP3 = 3, STEP4 = 4, STEP5 = 5, STEP6 = 6 } progress_ = NONE;

  // void (MiotYLxx0xYLNode::*step)();
  // uint16_t step_handle_ = {};
  // void do_step_(uint16_t handle);
  void step1_();
  void step2_();
  void step3_();
  void step5_();
  void step4_(const uint8_t *data, uint16_t size);
  void step6_();
};

}  // namespace miot_client
}  // namespace esphome
