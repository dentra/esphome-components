#include "esphome/core/log.h"
#include <array>
#include "miot_legacy_cipher.h"
#include "miot_legacy_bond_client.h"

namespace esphome {
namespace miot_client {

static const char *const TAG = "miot_legacy_bond_client";

// https://github.com/freenetwork/ble-in-xiaomi/blob/master/doc/auth_protocol.md
constexpr uint32_t OPCODE_LOGIN_START = 0xcd43bc00;
constexpr uint32_t CMD_LOGIN_DONE = 0x93bfac09;

constexpr uint32_t OPCODE_REG_START = 0xde85ca90;
constexpr uint32_t CMD_BONDING_DONE = 0xfa54ab92;

void MiotLegacyBondClient::on_disconnect(const esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param &param) {
  this->char_ = {};
  if (this->progress_ != NONE) {
    ESP_LOGW(TAG, "Bonding aborted due to device disconnect");
    this->progress_ = NONE;
  }
}

void MiotLegacyBondClient::on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) {
  this->char_.ctrlp = this->get_char_handle(miot_client::BLE_UUID_MI_SERVICE, miot_client::BLE_UUID_MI_CONTROL_POINT);
  if (this->char_.ctrlp == ESP_GATT_ILLEGAL_HANDLE) {
    return;
  }
  this->char_.auth = this->get_char_handle(miot_client::BLE_UUID_MI_SERVICE, miot_client::BLE_UUID_MI_LEGACY_AUTH);
  if (this->char_.auth == ESP_GATT_ILLEGAL_HANDLE) {
    return;
  }
  this->char_.ver = this->get_char_handle(miot_client::BLE_UUID_MI_SERVICE, miot_client::BLE_UUID_MI_VERSION);
  if (this->char_.ver == ESP_GATT_ILLEGAL_HANDLE) {
    return;
  }

  this->start_auth();
}

void MiotLegacyBondClient::on_write_char(const esp_ble_gattc_cb_param_t::gattc_write_evt_param &param) {
  if (this->progress_ == STEP1 && param.handle == this->char_.ctrlp) {
    ESP_LOGD(TAG, "Step %d complete: %s", this->progress_, YESNO(param.status == ESP_GATT_OK));
    this->step2_();
  } else if (this->progress_ == STEP5 && param.handle == this->char_.auth) {
    ESP_LOGD(TAG, "Step %d complete: %s", this->progress_, YESNO(param.status == ESP_GATT_OK));
    this->step6_();
  } else if (this->progress_ == STEP3 && param.handle == this->char_.auth) {
    ESP_LOGD(TAG, "Step %d complete: %s", this->progress_, YESNO(param.status == ESP_GATT_OK));
    // now waiting for notification
  }
}

void MiotLegacyBondClient::on_register_for_notification(
    const esp_ble_gattc_cb_param_t::gattc_reg_for_notify_evt_param &param) {
  if (this->progress_ == STEP2 && param.handle == this->char_.auth) {
    ESP_LOGD(TAG, "Step %d complete: %s", this->progress_, YESNO(param.status == ESP_GATT_OK));
    this->step3_();
  }
}

void MiotLegacyBondClient::on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) {
  if (this->progress_ == STEP6 && param.handle == this->char_.ver) {
    ESP_LOGD(TAG, "Step %d complete: %s", this->progress_, YESNO(param.status == ESP_GATT_OK));
    this->progress_ = NONE;
    this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
    ESP_LOGD(TAG, "Bonding complete");
    this->fire_auth_complete_();
  }
}

void MiotLegacyBondClient::on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) {
  if (this->progress_ == STEP3 && param.handle == this->char_.auth) {
    this->step4_(param.value, param.value_len);
  }
}

void MiotLegacyBondClient::start_auth() {
  if (this->progress_ != NONE) {
    ESP_LOGW(TAG, "Bonding already started");
    return;
  }
  this->token_ = cipher::generate_random_token();
  ESP_LOGD(TAG, "Bonding started, token: %s", format_hex(this->token_.data(), this->token_.size()).c_str());
  this->step1_();
}

void MiotLegacyBondClient::step1_() {
  this->progress_ = STEP1;
  ESP_LOGD(TAG, "Step %d running", this->progress_);
  this->write_char(this->char_.ctrlp, OPCODE_REG_START, true);
}

void MiotLegacyBondClient::step2_() {
  this->progress_ = STEP2;
  ESP_LOGD(TAG, "Step %d running", this->progress_);
  this->register_for_notify(this->char_.auth);
}

void MiotLegacyBondClient::step3_() {
  this->progress_ = STEP3;
  ESP_LOGD(TAG, "Step %d running", this->progress_);
  auto step3 = cipher::cipher(cipher::mix_a(this->parent()->get_address(), this->product_id_), this->token_);
  this->write_char(this->char_.auth, step3, true);
}

void MiotLegacyBondClient::step4_(const uint8_t *data, uint16_t size) {
  const auto result =
      cipher::cipher(cipher::mix_b(this->parent()->get_address(), this->product_id_),
                     cipher::cipher(cipher::mix_a(this->parent()->get_address(), this->product_id_), data, size));

  const bool complete =
      this->token_.size() == result.size() && memcmp(this->token_.data(), result.data(), result.size()) == 0;
  ESP_LOGD(TAG, "Step %d complete: %s", this->progress_, YESNO(complete));

  if (complete) {
    this->step5_();
  } else {
    // bonding failed, reset progress
    this->progress_ = NONE;
  }
}

void MiotLegacyBondClient::step5_() {
  this->progress_ = STEP5;
  ESP_LOGD(TAG, "Step %d running", this->progress_);
  auto step5 = cipher::cipher(this->token_, CMD_BONDING_DONE);
  this->write_char(this->char_.auth, step5, true);
}

void MiotLegacyBondClient::step6_() {
  this->progress_ = STEP6;
  ESP_LOGD(TAG, "Step %d running", this->progress_);
  this->read_char(this->char_.ver);
}

optional<std::vector<uint8_t>> MiotLegacyBondClient::decode(const uint8_t *data, const uint16_t size) const {
  if (this->debug_) {
    ESP_LOGD(TAG, "Decoding %s", format_hex_pretty(data, size).c_str());
  }
  return cipher::cipher(this->token_, data, size);
}

optional<std::vector<uint8_t>> MiotLegacyBondClient::encode(const uint8_t *data, const uint16_t size) {
  if (this->debug_) {
    ESP_LOGD(TAG, "Encoding %s", format_hex_pretty(data, size).c_str());
  }
  return cipher::cipher(this->token_, data, size);
}

}  // namespace miot_client
}  // namespace esphome
