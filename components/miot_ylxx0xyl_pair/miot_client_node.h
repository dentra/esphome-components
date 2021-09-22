#pragma once

#include "esphome/components/ble_client/ble_client.h"

namespace esphome {
namespace miot_client {

class MiotClientNode : public ble_client::BLEClientNode {
 public:
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  virtual void on_open(const esp_ble_gattc_cb_param_t::gattc_open_evt_param &param) {}
  virtual void on_disconnect(const esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param &param) {}
  virtual void on_register_for_notification(const esp_ble_gattc_cb_param_t::gattc_reg_for_notify_evt_param &param) {}
  virtual void on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) {}
  virtual void on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) {}
  virtual void on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) {}
  virtual void on_write_char(const esp_ble_gattc_cb_param_t::gattc_write_evt_param &param) {}
  virtual void on_write_descr(const esp_ble_gattc_cb_param_t::gattc_write_evt_param &param) {}

  bool subscribeCharacteristic(uint16_t handle);
  bool readCharacteristic(uint16_t handle);
  bool writeCharacteristic(uint16_t handle, const std::vector<uint8_t> &data, bool need_response = false);
  bool writeCharacteristic(uint16_t handle, const uint8_t *data, int size, bool need_response = false);

  /**
   * Get MIOT service (0xFE95).
   * @return pointer to service or nullptr on error
   **/
  ble_client::BLEService *get_miot_service() {
    return this->parent()->get_service(esp32_ble_tracker::ESPBTUUID::from_uint16(0xFE95));
  }

 protected:
};

}  // namespace miot_client
}  // namespace esphome
