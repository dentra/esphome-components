#include "esphome/core/defines.h"
#ifdef USE_VPORT_BLE
#include "esphome/core/log.h"
#include "esphome/core/version.h"
#include "esphome/core/application.h"
#include "vport_ble.h"

namespace esphome {
namespace vport {

static const char *const TAG = "vport_ble";

#if ESPHOME_VERSION_CODE < VERSION_CODE(2025, 12, 0)
#define ble_client_address_str(client) client->address_str().c_str()
#else
#define ble_client_address_str(client) client->address_str()
#endif

void VPortBLENode::dump_settings(const char *tag) const {
  ESP_LOGCONFIG(tag, "  MAC Address: %s", ble_client_address_str(this->parent_));
#if ESPHOME_VERSION_CODE < VERSION_CODE(2026, 2, 0)
  ESP_LOGCONFIG(tag, "  BLE Service: %s", this->ble_service_.to_string().c_str());
  ESP_LOGCONFIG(tag, "  BLE Char TX: %s", this->ble_char_tx_.to_string().c_str());
  ESP_LOGCONFIG(tag, "  BLE Char RX: %s", this->ble_char_rx_.to_string().c_str());
#else
  char char_buf[esp32_ble::UUID_STR_LEN];
  ESP_LOGCONFIG(tag, "  BLE Service: %s", this->ble_service_.to_str(char_buf));
  ESP_LOGCONFIG(tag, "  BLE Char TX: %s", this->ble_char_tx_.to_str(char_buf));
  ESP_LOGCONFIG(tag, "  BLE Char RX: %s", this->ble_char_rx_.to_str(char_buf));
#endif
  ESP_LOGCONFIG(tag, "  BLE Sec Act: %u", this->ble_sec_act_);
  ESP_LOGCONFIG(tag, "  BLE Disconnect on Auth Fail: %s", YESNO(this->ble_disconnect_on_auth_fail_));
}

void VPortBLENode::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                       esp_ble_gattc_cb_param_t *param) {
  auto *ble_client = this->parent_;

  if (event == ESP_GATTC_NOTIFY_EVT) {
    ESP_LOGV(TAG, "[%s] Got notify for handle %04u: %s", ble_client_address_str(ble_client), param->notify.handle,
             format_hex_pretty(param->notify.value, param->notify.value_len).c_str());
    if (param->notify.handle == this->char_rx_) {
#ifdef USE_VPORT_BLE_RUN_LATER
      auto *data = new uint8_t[param->notify.value_len];
      std::memcpy(data, param->notify.value, param->notify.value_len);
      this->parent_->run_later([this, data, size = param->notify.value_len]() -> void {
        this->on_ble_data(data, size);
        delete[] data;
      });
#else
      this->on_ble_data(param->notify.value, param->notify.value_len);
#endif
    }
    return;
  }

  if (event == ESP_GATTC_WRITE_DESCR_EVT) {
    ESP_LOGV(TAG, "[%s] Complete write_char_descr to 0x%x, status=0x%02x", ble_client_address_str(ble_client),
             param->write.handle, param->write.status);
    ESP_LOGI(TAG, "Connection established");
    this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
    this->on_ble_ready();
    return;
  }

  if (event == ESP_GATTC_SEARCH_CMPL_EVT) {
    auto tx = ble_client->get_characteristic(this->ble_service_, this->ble_char_tx_);
    if (tx == nullptr) {
      ESP_LOGE(TAG, "[%s] Can't discover TX characteristics", ble_client_address_str(ble_client));
      this->disconnect();
      return;
    }
    this->char_tx_ = tx->handle;

    auto rx = ble_client->get_characteristic(this->ble_service_, this->ble_char_rx_);
    if (rx == nullptr) {
      ESP_LOGE(TAG, "[%s] Can't discover RX characteristics", ble_client_address_str(ble_client));
      this->disconnect();
      return;
    }
    this->char_rx_ = rx->handle;

    ESP_LOGD(TAG, "[%s] Discovering complete", ble_client_address_str(ble_client));
    ESP_LOGV(TAG, "[%s]  TX handle 0x%x", ble_client_address_str(ble_client), this->char_tx_);
    ESP_LOGV(TAG, "[%s]  RX handle 0x%x", ble_client_address_str(ble_client), this->char_rx_);

    if (this->ble_reg_for_notify()) {
      auto err =
          esp_ble_gattc_register_for_notify(ble_client->get_gattc_if(), ble_client->get_remote_bda(), this->char_rx_);
      ESP_LOGV(TAG, "[%s] Register for notify 0x%x complete: %s", ble_client_address_str(ble_client), this->char_rx_,
               YESNO(err == ESP_OK));
    } else {
      ESP_LOGI(TAG, "Connection established");
      this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
      this->on_ble_ready();
    }

    return;
  }

  if (event == ESP_GATTC_CONNECT_EVT) {
    // let the device to do pair
    if (this->ble_sec_act_ != 0) {
      auto err = esp_ble_set_encryption(param->connect.remote_bda, this->ble_sec_act_);
      ESP_LOGV(TAG, "[%s] Bonding complete: %s", ble_client_address_str(ble_client), YESNO(err == ESP_OK));
    } else {
      ESP_LOGV(TAG, "[%s] Bonding skipped", ble_client_address_str(ble_client));
    }
    if (this->disable_scan_) {
      esp32_ble_tracker::global_esp32_ble_tracker->stop_scan();
    }
    return;
  }

  if (event == ESP_GATTC_DISCONNECT_EVT) {
    ESP_LOGV(TAG, "Disconnected. Status: %d, reason: %d", param->close.status, param->close.reason);
    this->node_state = esp32_ble_tracker::ClientState::IDLE;
    if (this->disable_scan_) {
      esp32_ble_tracker::global_esp32_ble_tracker->start_scan();
    }
    return;
  }

#ifdef ESPHOME_LOG_HAS_VERBOSE
  if (event == ESP_GATTC_OPEN_EVT) {
    if (param->open.status != ESP_GATT_OK) {
      ESP_LOGW(TAG, "Connection fails. Status: %d", param->open.status);
    }
    return;
  }

  if (event == ESP_GATTC_REG_FOR_NOTIFY_EVT) {
    ESP_LOGV(TAG, "[%s] Registring for notify complete", ble_client_address_str(ble_client));
    return;
  }

  if (event == ESP_GATTC_WRITE_CHAR_EVT) {
    ESP_LOGV(TAG, "[%s] Complete write_char to 0x%x, status=0x%02x", ble_client_address_str(ble_client),
             param->write.handle, param->write.status);
    return;
  }

  if (event == ESP_GATTC_ENC_CMPL_CB_EVT) {
    ESP_LOGV(TAG, "[%s] Complete esp_ble_set_encryption", ble_client_address_str(ble_client));
    return;
  }
#endif
}

void VPortBLENode::gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
  if (!this->ble_disconnect_on_auth_fail_) {
    return;
  }
  if (event != ESP_GAP_BLE_AUTH_CMPL_EVT || param->ble_security.auth_cmpl.success) {
    return;
  }
  if (!this->parent_->check_addr(param->ble_security.auth_cmpl.bd_addr)) {
    return;
  }
  // connection stays open after failed auth, but device does not respond, so drop it and reconnect later
  ESP_LOGW(TAG, "[%s] Auth failed, reason=%u, disconnecting", ble_client_address_str(this->parent_),
           param->ble_security.auth_cmpl.fail_reason);
  this->disconnect();
}

bool VPortBLENode::write_ble_data(const uint8_t *data, uint16_t size) const {
  if (!this->is_connected()) {
    ESP_LOGW(TAG, "[%s] Not connected, can't write: %s", ble_client_address_str(this->parent_),
             format_hex_pretty(data, size).c_str());
    return false;
  }

  auto *ble_client = this->parent_;
  ESP_LOGV(TAG, "[%s] Writing data to 0x%x: %s", ble_client_address_str(ble_client), this->char_tx_,
           format_hex_pretty(data, size).c_str());
#ifdef ESPHOME_LOG_HAS_VERBOSE
  auto write_type = ESP_GATT_WRITE_TYPE_RSP;
#else
  auto write_type = ESP_GATT_WRITE_TYPE_NO_RSP;
#endif
  return esp_ble_gattc_write_char(ble_client->get_gattc_if(), ble_client->get_conn_id(), this->char_tx_, size,
                                  const_cast<uint8_t *>(data), write_type, ESP_GATT_AUTH_REQ_NONE) == ESP_OK;
}

void VPortBLENode::connect() {
  if (this->node_state == esp32_ble_tracker::ClientState::CONNECTING) {
    return;
  }

  if (this->node_state == esp32_ble_tracker::ClientState::ESTABLISHED) {
    return;
  }

  this->node_state = esp32_ble_tracker::ClientState::CONNECTING;
  this->parent()->set_enabled(true);
}

void VPortBLENode::disconnect() {
  if (this->node_state == esp32_ble_tracker::ClientState::IDLE) {
    return;
  }
  this->parent()->set_enabled(false);
  // additionally clear memory if connection was dropped
  this->parent()->release_services();
  this->node_state = esp32_ble_tracker::ClientState::IDLE;
}

}  // namespace vport
}  // namespace esphome
#endif  // USE_VPORT_BLE
