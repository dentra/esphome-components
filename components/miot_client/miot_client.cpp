
#include "esphome/core/log.h"
#include "miot_client.h"

namespace esphome {
namespace miot_client {

static const char *const TAG = "miot_client";

inline void GATTC_LOG(const char *event, const char *param, int value) {
  ESP_LOGV(TAG, "%s %s, %s=0x%X", "Got gattc event", event, param, value);
}

inline void GATC_ERR(const char *param, esp_gatt_status_t status) {
  ESP_LOGW(TAG, "Error %s, status=0x%02X", param, status);
}

inline void GATC_ERR(const char *param, uint16_t handle, esp_gatt_status_t status) {
  ESP_LOGW(TAG, "Error %s at handle 0x%X, status=0x%02X", param, handle, status);
}

inline void GATC_ERR(const char *param, const esp_bd_addr_t &addr) {
  ESP_LOGW(TAG, "Remote address for %s does not match client address", param);
}

inline void GATC_ERR(const char *param, const std::string &mac, const esp_err_t &err) {
  ESP_LOGW(TAG, "[%s] %s failed, status=0x%02X ", mac.c_str(), param, err);
}

void MiotClient::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                     esp_ble_gattc_cb_param_t *param) {
  if (event == ESP_GATTC_OPEN_EVT) {
    GATTC_LOG("ESP_GATTC_OPEN_EVT", "status", param->open.status);
    if (param->open.conn_id != this->parent()->conn_id) {
      return;
    }
    if (param->open.status != ESP_GATT_OK) {
      GATC_ERR("open", param->open.status);
      return;
    }
    if (memcmp(param->open.remote_bda, this->parent()->remote_bda, sizeof(esp_bd_addr_t)) != 0) {
      GATC_ERR("open", param->open.remote_bda);
      return;
    }
    this->on_open(param->open);
    return;
  }

  if (event == ESP_GATTC_DISCONNECT_EVT) {
    GATTC_LOG("ESP_GATTC_DISCONNECT_EVT", "reason", param->disconnect.reason);
    if (param->disconnect.conn_id != this->parent()->conn_id) {
      return;
    }
    if (memcmp(param->disconnect.remote_bda, this->parent()->remote_bda, sizeof(esp_bd_addr_t)) != 0) {
      GATC_ERR("disconnect", param->disconnect.remote_bda);
      return;
    }
    this->on_disconnect(param->disconnect);
    this->node_state = esp32_ble_tracker::ClientState::IDLE;
    return;
  }

  if (event == ESP_GATTC_SEARCH_CMPL_EVT) {
    GATTC_LOG("ESP_GATTC_SEARCH_CMPL_EVT", "status", param->search_cmpl.status);
    if (param->search_cmpl.conn_id != this->parent()->conn_id) {
      return;
    }
    if (param->search_cmpl.status != ESP_GATT_OK) {
      GATC_ERR("search_cmpl", param->search_cmpl.status);
      return;
    }
    this->on_search_complete(param->search_cmpl);
    return;
  }

  if (event == ESP_GATTC_REG_FOR_NOTIFY_EVT) {
    GATTC_LOG("ESP_GATTC_REG_FOR_NOTIFY_EVT", "handle", param->reg_for_notify.handle);
    if (param->reg_for_notify.status != ESP_GATT_OK) {
      GATC_ERR("reg_for_notify", param->reg_for_notify.status);
      return;
    }
    this->on_register_for_notification(param->reg_for_notify);
    return;
  }

  if (event == ESP_GATTC_READ_CHAR_EVT) {
    GATTC_LOG("ESP_GATTC_READ_CHAR_EVT", "handle", param->read.handle);
    if (param->read.conn_id != this->parent()->conn_id) {
      return;
    }
    if (param->read.status != ESP_GATT_OK) {
      GATC_ERR("read_char", param->read.handle, param->read.status);
      return;
    }
    const auto is_encrypted = this->is_encrypted_(param->read.handle);
    ESP_LOGV(TAG, "Got %s%s for 0x%X, data: %s", "read_char", is_encrypted ? " (encrypted)" : "", param->read.handle,
             format_hex_pretty(param->read.value, param->read.value_len).c_str());
    if (is_encrypted) {
      esp_ble_gattc_cb_param_t::gattc_read_char_evt_param copy;
      memcpy(&copy, &param->read, sizeof(esp_ble_gattc_cb_param_t::gattc_read_char_evt_param));
      auto value = this->auth_->decode(param->read.value, param->read.value_len);
      if (value.has_value()) {
        copy.value = value->data();
        copy.value_len = value->size();
        this->on_read_char(copy);
      } else {
        ESP_LOGW(TAG, "Decryption failed: %s", format_hex_pretty(param->read.value, param->read.value_len).c_str());
      }
    } else {
      this->on_read_char(param->read);
    }
    return;
  }

  if (event == ESP_GATTC_NOTIFY_EVT) {
    GATTC_LOG("ESP_GATTC_NOTIFY_EVT", "handle", param->notify.handle);
    if (param->notify.conn_id != this->parent()->conn_id) {
      return;
    }
    if (memcmp(param->notify.remote_bda, this->parent()->remote_bda, sizeof(esp_bd_addr_t)) != 0) {
      GATC_ERR("notify", param->notify.remote_bda);
      return;
    }
    const auto is_encrypted = this->is_encrypted_(param->read.handle);
    ESP_LOGV(TAG, "Got %s%s for 0x%X, data: %s", "notify", is_encrypted ? " (encrypted)" : "", param->notify.handle,
             format_hex_pretty(param->notify.value, param->notify.value_len).c_str());
    if (this->is_encrypted_(param->notify.handle)) {
      esp_ble_gattc_cb_param_t::gattc_notify_evt_param copy;
      memcpy(&copy, &param->notify, sizeof(esp_ble_gattc_cb_param_t::gattc_notify_evt_param));
      auto value = this->auth_->decode(param->notify.value, param->notify.value_len);
      if (value.has_value()) {
        copy.value = value->data();
        copy.value_len = value->size();
        this->on_notify(copy);
      } else {
        ESP_LOGW(TAG, "Decryption failed: %s", format_hex_pretty(param->notify.value, param->notify.value_len).c_str());
      }
    } else {
      this->on_notify(param->notify);
    }
    return;
  }

  if (event == ESP_GATTC_WRITE_CHAR_EVT) {
    GATTC_LOG("ESP_GATTC_WRITE_CHAR_EVT", "handle", param->write.handle);
    if (param->write.conn_id != this->parent()->conn_id) {
      return;
    }
    if (param->write.status != ESP_GATT_OK) {
      GATC_ERR("write_char", param->write.handle, param->write.status);
      return;
    }
    this->on_write_char(param->write);
    return;
  }

  if (event == ESP_GATTC_WRITE_DESCR_EVT) {
    GATTC_LOG("ESP_GATTC_WRITE_DESCR_EVT", "handle", param->write.handle);
    if (param->write.conn_id != this->parent()->conn_id) {
      return;
    }
    if (param->write.status != ESP_GATT_OK) {
      GATC_ERR("write_descr", param->write.handle, param->write.status);
      return;
    }
    this->on_write_descr(param->write);
    return;
  }

  ESP_LOGV(TAG, "Got gattc event [%d]", event);
}

bool MiotClient::register_for_notify(uint16_t handle) const {
  ESP_LOGV(TAG, "esp_ble_gattc_register_for_notify, handle=0x%X", handle);
  auto status = esp_ble_gattc_register_for_notify(this->parent_->gattc_if, this->parent_->remote_bda, handle);
  if (status == ESP_GATT_OK) {
    return true;
  }
  GATC_ERR("esp_ble_gattc_register_for_notify", this->parent_->address_str(), status);
  return false;
}

bool MiotClient::read_char(uint16_t handle) const {
  ESP_LOGV(TAG, "esp_ble_gattc_read_char, handle=0x%X", handle);

  auto status =
      esp_ble_gattc_read_char(this->parent_->gattc_if, this->parent_->conn_id, handle, ESP_GATT_AUTH_REQ_NONE);
  if (status == ESP_GATT_OK) {
    return true;
  }
  GATC_ERR("esp_ble_gattc_read_char", this->parent_->address_str(), status);
  return false;
}

bool MiotClient::write_char(uint16_t handle, const uint8_t *data, uint16_t size, bool need_response) const {
  ESP_LOGV(TAG, "esp_ble_gattc_write_char, handle=0x%X, data: %s", handle, format_hex_pretty(data, size).c_str());

  esp_err_t status;
  if (this->is_encrypted_(handle)) {
    auto value = this->auth_->encode(data, size);
    if (!value.has_value()) {
      ESP_LOGW(TAG, "Encryption failed: %s", format_hex_pretty(data, size).c_str());
      return false;
    }
    status = esp_ble_gattc_write_char(
        this->parent_->gattc_if, this->parent_->conn_id, handle, value->size(), value->data(),
        need_response ? ESP_GATT_WRITE_TYPE_RSP : ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
  } else {
    status = esp_ble_gattc_write_char(
        this->parent_->gattc_if, this->parent_->conn_id, handle, size, const_cast<uint8_t *>(data),
        need_response ? ESP_GATT_WRITE_TYPE_RSP : ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
  }

  if (status == ESP_GATT_OK) {
    return true;
  }

  GATC_ERR("esp_ble_gattc_write_char", this->parent_->address_str(), status);

  return false;
}

esphome::ble_client::BLECharacteristic *MiotClient::get_char(const esp32_ble_tracker::ESPBTUUID &srv,
                                                             const esp32_ble_tracker::ESPBTUUID &chr,
                                                             bool log_not_found) {
  auto res = this->parent()->get_characteristic(srv, chr);
  if (res == nullptr && log_not_found) {
    ESP_LOGE(TAG, "Can't discover characteristics %s at service %s", chr.to_string().c_str(), srv.to_string().c_str());
  }
  return res;
}

}  // namespace miot_client
}  // namespace esphome
