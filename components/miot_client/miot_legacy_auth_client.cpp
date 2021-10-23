#include "esphome/core/log.h"
#include <array>
#include "miot_legacy_auth_cipher.h"
#include "miot_legacy_auth_client.h"

namespace esphome {
namespace miot_client {

static const char *const TAG = "miot_legacy_auth_client";

constexpr auto BONDING_INIT = std::array<uint8_t, 4>{0x90, 0xCA, 0x85, 0xDE};
constexpr auto BONDING_DONE = std::array<uint8_t, 4>{0x92, 0xAB, 0x54, 0xFA};

void MiotLegacyAuthClient::on_disconnect(const esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param &param) {
  this->char_ = {};
  if (this->progress_ != NONE) {
    ESP_LOGW(TAG, "Bonding aborted due to device disconnect");
    this->progress_ = NONE;
  }
}

void MiotLegacyAuthClient::on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) {
  this->char_.ctrlp =
      this->get_characteristic_handle(miot_client::BT_UUID_MI_SERVICE, miot_client::BT_UUID_MI_CONTROL_POINT);
  if (this->char_.ctrlp == ESP_GATT_ILLEGAL_HANDLE) {
    ESP_LOGE(TAG, "Can't discover characteristics %04X", miot_client::BT_UUID_MI_CONTROL_POINT);
    return;
  }

  this->char_.auth =
      this->get_characteristic_handle(miot_client::BT_UUID_MI_SERVICE, miot_client::BT_UUID_MI_LEGACY_AUTH);
  if (this->char_.auth == ESP_GATT_ILLEGAL_HANDLE) {
    ESP_LOGE(TAG, "Can't discover characteristics %04X", miot_client::BT_UUID_MI_LEGACY_AUTH);
    return;
  }

  this->char_.ver = this->get_characteristic_handle(miot_client::BT_UUID_MI_SERVICE, miot_client::BT_UUID_MI_VERSION);
  if (this->char_.ver == ESP_GATT_ILLEGAL_HANDLE) {
    ESP_LOGE(TAG, "Can't discover characteristics %04X", miot_client::BT_UUID_MI_VERSION);
    return;
  }

  this->start_auth();
}

void MiotLegacyAuthClient::on_write_char(const esp_ble_gattc_cb_param_t::gattc_write_evt_param &param) {
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

void MiotLegacyAuthClient::on_register_for_notification(
    const esp_ble_gattc_cb_param_t::gattc_reg_for_notify_evt_param &param) {
  if (this->progress_ == STEP2 && param.handle == this->char_.auth) {
    ESP_LOGD(TAG, "Step %d complete: %s", this->progress_, YESNO(param.status == ESP_GATT_OK));
    this->step3_();
  }
}

void MiotLegacyAuthClient::on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) {
  if (this->progress_ == STEP6 && param.handle == this->char_.ver) {
    ESP_LOGD(TAG, "Step %d complete: %s", this->progress_, YESNO(param.status == ESP_GATT_OK));
    this->progress_ = NONE;
    this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
    ESP_LOGD(TAG, "Bonding complete");
    this->fire_auth_complete_();
  }
}

void MiotLegacyAuthClient::on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) {
  if (this->progress_ == STEP3 && param.handle == this->char_.auth) {
    this->step4_(param.value, param.value_len);
  }
}

void MiotLegacyAuthClient::start_auth() {
  if (this->progress_ != NONE) {
    ESP_LOGW(TAG, "Bonding already started");
    return;
  }
  this->token_ = cipher::generate_random_token();
  ESP_LOGD(TAG, "Bonding started, token: %s", hexencode(this->token_).c_str());
  this->step1_();
}

void MiotLegacyAuthClient::step1_() {
  this->progress_ = STEP1;
  ESP_LOGD(TAG, "Step %d running", this->progress_);
  this->write_char(this->char_.ctrlp, BONDING_INIT, true);
}

void MiotLegacyAuthClient::step2_() {
  this->progress_ = STEP2;
  ESP_LOGD(TAG, "Step %d running", this->progress_);
  this->register_for_notify(this->char_.auth);
}

void MiotLegacyAuthClient::step3_() {
  this->progress_ = STEP3;
  ESP_LOGD(TAG, "Step %d running", this->progress_);
  auto step3 = cipher::cipher(cipher::mix_a(this->parent()->address, this->product_id_), this->token_);
  this->write_char(this->char_.auth, step3, true);
}

void MiotLegacyAuthClient::step4_(const uint8_t *data, uint16_t size) {
  const auto result =
      cipher::cipher(cipher::mix_b(this->parent()->address, this->product_id_),
                     cipher::cipher(cipher::mix_a(this->parent()->address, this->product_id_), data, size));

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

void MiotLegacyAuthClient::step5_() {
  this->progress_ = STEP5;
  ESP_LOGD(TAG, "Step %d running", this->progress_);
  auto step5 = cipher::cipher(this->token_, BONDING_DONE);
  this->write_char(this->char_.auth, step5, true);
}

void MiotLegacyAuthClient::step6_() {
  this->progress_ = STEP6;
  ESP_LOGD(TAG, "Step %d running", this->progress_);
  this->read_char(this->char_.ver);
}

bool MiotLegacyAuthClient::is_auth_complete() const {
  return this->node_state == esp32_ble_tracker::ClientState::ESTABLISHED;
}

std::vector<uint8_t> MiotLegacyAuthClient::decode(const uint8_t *data, const uint16_t size) const {
  if (this->debug_) {
    ESP_LOGD(TAG, "Decoding %s", hexencode(data, size).c_str());
  }
  return cipher::cipher(this->token_, data, size);
}

std::vector<uint8_t> MiotLegacyAuthClient::encode(const uint8_t *data, const uint16_t size) const {
  if (this->debug_) {
    ESP_LOGD(TAG, "Encoding %s", hexencode(data, size).c_str());
  }
  return cipher::cipher(this->token_, data, size);
}

}  // namespace miot_client
}  // namespace esphome
