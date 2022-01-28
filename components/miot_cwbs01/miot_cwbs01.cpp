#include "esphome/core/log.h"
#include "miot_cwbs01.h"
namespace esphome {
namespace miot_cwbs01 {

static const char *const TAG = "miot_cwbs01";

void MiotCWBS01::on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) {
  this->char_.version = this->get_char_handle(miot_client::BLE_UUID_MI_SERVICE, miot_client::BLE_UUID_MI_VERSION);
  if (this->char_.version == ESP_GATT_ILLEGAL_HANDLE) {
    return;
  }
  this->char_.stdio_tx =
      this->get_char_handle(miot_client::BLE_UUID_MI_STDIO_SERVICE, miot_client::BLE_UUID_MI_STDIO_RX);
  if (this->char_.stdio_tx == ESP_GATT_ILLEGAL_HANDLE) {
    return;
  }
  this->char_.stdio_rx =
      this->get_char_handle(miot_client::BLE_UUID_MI_STDIO_SERVICE, miot_client::BLE_UUID_MI_STDIO_TX);
  if (this->char_.stdio_rx == ESP_GATT_ILLEGAL_HANDLE) {
    return;
  }

  this->register_for_notify(this->char_.stdio_rx);

  this->add_auth_char(this->char_.stdio_rx);
  this->add_auth_char(this->char_.stdio_tx);
}

void MiotCWBS01::on_auth_complete() {
  if (this->version_ != nullptr) {
    this->read_char(this->char_.version);
  }
  this->enable_state_reporting();
}

void MiotCWBS01::on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) {
  if (param.handle == this->char_.version) {
    if (this->version_ != nullptr) {
      this->version_->publish_state(std::string(reinterpret_cast<char const *>(param.value), param.value_len));
    }
  }
}

void MiotCWBS01::on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) {
  if (param.handle == this->char_.stdio_rx) {
    this->read_frame(param.value, param.value_len);
  }
}

void MiotCWBS01::read(const datetime_sync_t &dts) {
  if (dts.sync == datetime_sync_t::DO_SYNC) {
    auto time = this->rtc_->now();
    if (time.is_valid()) {
      this->sync_time(time);
    }
  }
}

void MiotCWBS01::read(const state_t &state) {
  this->publish_state(state.state);
  if (this->power_ != nullptr) {
    this->power_->publish_state(state.power == State::STATE_ON);
  }
  if (this->mode_ != nullptr) {
    this->mode_->publish_state(this->mode_->get_option(state.mode));
  }
  if (this->cycle_ != nullptr) {
    this->cycle_->publish_state(state.cycle == Cycle::CYCLE_ON);
  }
  if (this->scene_ != nullptr) {
    this->scene_->publish_state(this->scene_->get_option(state.scene));
  }
  if (this->charging_ != nullptr) {
    this->charging_->publish_state(state.power_state == PowerState::POWER_STATE_CHARGING);
  }
  if (this->error_ != nullptr) {
    this->error_->publish_state(state.error != 0);
  }
  if (this->battery_level_ != nullptr) {
    this->battery_level_->publish_state(state.battery);
  }
}

}  // namespace miot_cwbs01
}  // namespace esphome
