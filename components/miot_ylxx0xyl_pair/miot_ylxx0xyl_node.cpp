#include "esphome/core/log.h"
#include "../miot/miot_utils.h"
#include "miot_ylxx0xyl_node.h"

namespace esphome {
namespace miot_ylxx0xyl {

static const char *const TAG = "miot_ylxx0xyl_node";

void MiotYLxx0xYLNode::on_disconnect(const esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param &param) {
  this->char_ = {};
  // disable auto reconnect
  this->parent()->set_enabled(false);
}

void MiotYLxx0xYLNode::on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) {
  this->char_.ver = this->get_char_handle(miot_client::BLE_UUID_MI_SERVICE, miot_client::BLE_UUID_MI_VERSION);
  if (this->char_.ver == ESP_GATT_ILLEGAL_HANDLE) {
    return;
  }
  this->char_.key = this->get_char_handle(miot_client::BLE_UUID_MI_SERVICE, miot_client::BLE_UUID_MI_BEACONKEY);
  if (this->char_.key == ESP_GATT_ILLEGAL_HANDLE) {
    return;
  }

  this->add_auth_char(this->char_.ver);
  this->add_auth_char(this->char_.key);
}

void MiotYLxx0xYLNode::on_auth_complete() {
  this->read_char(this->char_.key);
  this->read_char(this->char_.ver);
}

void MiotYLxx0xYLNode::on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) {
  if (this->auth_->is_auth_complete()) {
    if (param.handle == this->char_.ver) {
      this->on_read_ver_(param.value, param.value_len);
    } else if (param.handle == this->char_.key) {
      this->on_read_key_(param.value, param.value_len);
    }
  }
}

void MiotYLxx0xYLNode::on_read_ver_(const uint8_t *data, int size) {
  this->publisher_->publish_version(std::string(reinterpret_cast<char const *>(data), size));
}

void MiotYLxx0xYLNode::on_read_key_(const uint8_t *data, int size) {
  this->publisher_->publish_bindkey(miot::hexstr(data, size));

  this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
  this->parent()->set_enabled(false);
}

}  // namespace miot_ylxx0xyl
}  // namespace esphome
