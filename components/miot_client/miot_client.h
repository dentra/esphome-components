#pragma once
#include <esp_gattc_api.h>
#include <set>
#include "esphome/components/ble_client/ble_client.h"
#include "miot_client_descr.h"

namespace esphome {
namespace miot_client {

class AuthClientListener {
 public:
  virtual void on_auth_complete() = 0;
};

class AuthClient {
 public:
  virtual std::vector<uint8_t> decode(const uint8_t *data, const uint16_t size) const = 0;
  virtual std::vector<uint8_t> encode(const uint8_t *data, const uint16_t size) const = 0;
  virtual bool is_auth_complete() const = 0;
  void add_auth_listener(AuthClientListener *listener) { this->auth_listeners_.push_back(listener); }
  void add_auth_char(uint16_t handle) {
    if (handle != ESP_GATT_ILLEGAL_HANDLE) {
      this->auth_char_.insert(handle);
    }
  }
  bool is_encrypted(uint16_t handle) const {
    return this->is_auth_complete() && this->auth_char_.find(handle) != this->auth_char_.end();
  }

 protected:
  void fire_auth_complete_() const {
    for (auto listener : this->auth_listeners_) {
      listener->on_auth_complete();
    }
  }
  std::vector<AuthClientListener *> auth_listeners_;
  std::set<uint16_t> auth_char_;
};

class MiotClient : public ble_client::BLEClientNode {
 public:
  void set_debug(bool debug) { this->debug_ = debug; }
  void set_auth_client(AuthClient *auth) { this->auth_ = auth; }
  void add_auth_char(uint16_t handle) {
    if (this->auth_ != nullptr) {
      this->auth_->add_auth_char(handle);
    }
  }

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  virtual void on_open(const esp_ble_gattc_cb_param_t::gattc_open_evt_param &param) {}
  virtual void on_disconnect(const esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param &param) {}
  virtual void on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) {}
  virtual void on_register_for_notification(const esp_ble_gattc_cb_param_t::gattc_reg_for_notify_evt_param &param) {}
  virtual void on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) {}
  virtual void on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) {}
  virtual void on_write_char(const esp_ble_gattc_cb_param_t::gattc_write_evt_param &param) {}
  virtual void on_write_descr(const esp_ble_gattc_cb_param_t::gattc_write_evt_param &param) {}

  // esp_ble_gattc_register_for_notify
  bool register_for_notify(uint16_t handle) const;
  // esp_ble_gattc_read_char
  bool read_char(uint16_t handle) const;
  // esp_ble_gattc_write_char
  template<typename T> bool write_char(uint16_t handle, const T &data, bool need_response = false) const {
    return write_char(handle, data.data(), data.size(), need_response);
  }
  // esp_ble_gattc_write_char
  bool write_char(uint16_t handle, const uint8_t *data, uint16_t size, bool need_response = false) const;

  bool is_established() const { return this->node_state == esp32_ble_tracker::ClientState::Established; }

  esphome::ble_client::BLECharacteristic *get_characteristic(const uint16_t service, const uint16_t characteristic) {
    return this->parent()->get_characteristic(esp32_ble_tracker::ESPBTUUID::from_uint16(service),
                                              esp32_ble_tracker::ESPBTUUID::from_uint16(characteristic));
  }

  esphome::ble_client::BLECharacteristic *get_characteristic(const esp_bt_uuid_t &service, uint16_t characteristic) {
    return this->parent()->get_characteristic(esp32_ble_tracker::ESPBTUUID::from_uuid(service),
                                              esp32_ble_tracker::ESPBTUUID::from_uint16(characteristic));
  }

  esphome::ble_client::BLECharacteristic *get_characteristic(const esp_bt_uuid_t &service,
                                                             const esp_bt_uuid_t &characteristic) {
    return this->parent()->get_characteristic(esp32_ble_tracker::ESPBTUUID::from_uuid(service),
                                              esp32_ble_tracker::ESPBTUUID::from_uuid(characteristic));
  }

  uint16_t get_characteristic_handle(const uint16_t service, const uint16_t characteristic) {
    auto chr = this->get_characteristic(service, characteristic);
    return chr != nullptr ? chr->handle : ESP_GATT_ILLEGAL_HANDLE;
  }

  uint16_t get_characteristic_handle(const esp_bt_uuid_t &service, uint16_t characteristic) {
    auto chr = this->get_characteristic(service, characteristic);
    return chr != nullptr ? chr->handle : ESP_GATT_ILLEGAL_HANDLE;
  }

  uint16_t get_characteristic_handle(const esp_bt_uuid_t &service, const esp_bt_uuid_t &characteristic) {
    auto chr = this->get_characteristic(service, characteristic);
    return chr != nullptr ? chr->handle : ESP_GATT_ILLEGAL_HANDLE;
  }

 protected:
  bool debug_ = {};
  AuthClient *auth_ = {};
  bool is_encrypted_(uint16_t handle) const { return this->auth_ != nullptr && this->auth_->is_encrypted(handle); }
};

}  // namespace miot_client
}  // namespace esphome
