
#include "esphome/core/log.h"
#include "miot_client_node.h"

namespace esphome {
namespace miot_client {

static const char *const TAG = "miot_client_node";

inline void GATTC_LOG(const char *event, const char *param, int value) {
  ESP_LOGD(TAG, "Got gattc event %s, %s=0x%x", event, param, value);
}

void MiotClientNode::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                         esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      GATTC_LOG("ESP_GATTC_OPEN_EVT", "status", param->open.status);
      if (param->open.conn_id != this->parent()->conn_id) {
        break;
      }
      if (param->open.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Error open, status=0x%02X", param->open.status);
        break;
      }
      if (memcmp(param->open.remote_bda, this->parent()->remote_bda, sizeof(esp_bd_addr_t)) != 0) {
        ESP_LOGW(TAG, "%s remote address does not match client address", "Open");
        break;
      }
      this->on_open(param->open);
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      GATTC_LOG("ESP_GATTC_DISCONNECT_EVT", "reason", param->disconnect.reason);
      if (param->disconnect.conn_id != this->parent()->conn_id) {
        break;
      }
      if (memcmp(param->disconnect.remote_bda, this->parent()->remote_bda, sizeof(esp_bd_addr_t)) != 0) {
        ESP_LOGW(TAG, "%s remote address does not match client address", "Disconnect");
        break;
      }
      this->on_disconnect(param->disconnect);
      this->node_state = esp32_ble_tracker::ClientState::Idle;
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      GATTC_LOG("ESP_GATTC_SEARCH_CMPL_EVT", "status", param->search_cmpl.status);
      if (param->search_cmpl.conn_id != this->parent()->conn_id) {
        break;
      }
      if (param->search_cmpl.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Error %s, status=0x%02X", "search_cmpl", param->search_cmpl.status);
        break;
      }
      this->node_state = esp32_ble_tracker::ClientState::Established;
      this->on_search_complete(param->search_cmpl);
      break;
    }
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      GATTC_LOG("ESP_GATTC_REG_FOR_NOTIFY_EVT", "handle", param->reg_for_notify.handle);
      if (param->reg_for_notify.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Error %s, status=0x%02X", "reg_for_notify", param->reg_for_notify.status);
        break;
      }
      this->on_register_for_notification(param->reg_for_notify);
      break;
    }
    case ESP_GATTC_READ_CHAR_EVT: {
      GATTC_LOG("ESP_GATTC_READ_CHAR_EVT", "handle", param->read.handle);
      if (param->read.conn_id != this->parent()->conn_id) {
        break;
      }
      if (param->read.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Error reading char at handle 0x%x, status=0x%02X", param->read.handle, param->read.status);
        break;
      }
      this->on_read_char(param->read);
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      GATTC_LOG("ESP_GATTC_NOTIFY_EVT", "handle", param->notify.handle);
      if (param->notify.conn_id != this->parent()->conn_id) {
        break;
      }
      if (memcmp(param->notify.remote_bda, this->parent()->remote_bda, sizeof(esp_bd_addr_t)) != 0) {
        ESP_LOGW(TAG, "%s remote address does not match client address", "Notify");
        break;
      }
      this->on_notify(param->notify);
      break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT: {
      GATTC_LOG("ESP_GATTC_WRITE_CHAR_EVT", "handle", param->write.handle);
      if (param->write.conn_id != this->parent()->conn_id) {
        break;
      }
      if (param->write.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Error writting char at handle 0x%x, status=0x%02X", param->write.handle, param->write.status);
        break;
      }
      this->on_write_char(param->write);
      break;
    }
    case ESP_GATTC_WRITE_DESCR_EVT: {
      GATTC_LOG("ESP_GATTC_WRITE_DESCR_EVT", "handle", param->write.handle);
      if (param->write.conn_id != this->parent()->conn_id) {
        break;
      }
      if (param->write.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Error writting descr at handle 0x%x, status=0x%02X", param->write.handle, param->write.status);
        break;
      }
      this->on_write_descr(param->write);
      break;
    }
    default: {
      ESP_LOGD(TAG, "Got gattc event [%d]", event);
      break;
    }
  }
}

bool MiotClientNode::subscribeCharacteristic(uint16_t handle) {
  ESP_LOGD(TAG, "esp_ble_gattc_register_for_notify, handle=0x%x", handle);
  auto status = esp_ble_gattc_register_for_notify(this->parent_->gattc_if, this->parent_->remote_bda, handle);
  if (status == ESP_GATT_OK) {
    return true;
  }
  ESP_LOGW(TAG, "[%s] esp_ble_gattc_register_for_notify failed, status=0x%02X", this->parent_->address_str().c_str(),
           status);
  return false;
}

bool MiotClientNode::readCharacteristic(uint16_t handle) {
  ESP_LOGD(TAG, "esp_ble_gattc_read_char, handle=0x%x", handle);
  auto status =
      esp_ble_gattc_read_char(this->parent_->gattc_if, this->parent_->conn_id, handle, ESP_GATT_AUTH_REQ_NONE);
  if (status == ESP_GATT_OK) {
    return true;
  }
  ESP_LOGW(TAG, "[%s] esp_ble_gattc_read_char failed, status=0x%02X ", this->parent_->address_str().c_str(), status);
  return false;
}

bool MiotClientNode::writeCharacteristic(uint16_t handle, const std::vector<uint8_t> &data, bool need_response) {
  return writeCharacteristic(handle, data.data(), data.size());
}

bool MiotClientNode::writeCharacteristic(uint16_t handle, const uint8_t *data, int size, bool need_response) {
  ESP_LOGD(TAG, "esp_ble_gattc_write_char, handle=0x%x, data: %s", handle, hexencode(data, size).c_str());
  auto status = esp_ble_gattc_write_char(
      this->parent_->gattc_if, this->parent_->conn_id, handle, size, (uint8_t *) data,
      need_response ? ESP_GATT_WRITE_TYPE_RSP : ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
  if (status == ESP_GATT_OK) {
    return true;
  }
  ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status=0x%02X ", this->parent_->address_str().c_str(), status);
  return false;
}

}  // namespace miot_client
}  // namespace esphome
