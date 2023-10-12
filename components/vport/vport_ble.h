#pragma once
#ifdef USE_VPORT_BLE

#include "esphome/components/ble_client/ble_client.h"
#include "vport_component.h"

namespace esphome {
namespace vport {

#define VPORT_BLE_LOG(port_name) \
  VPORT_LOG(port_name); \
  this->io_->dump_settings(TAG); \
  ESP_LOGCONFIG(TAG, "  Persistent connection: %s", ONOFF(this->persistent_connection_));

class VPortBLENode : public ble_client::BLEClientNode {
 public:
  void dump_settings(const char *tag) const;
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  virtual void on_ble_ready() = 0;
  virtual bool on_ble_data(const uint8_t *data, uint16_t size) = 0;
  bool write_ble_data(const uint8_t *data, uint16_t size) const;

  void set_ble_service(const char *ble_service) {
    this->ble_service_ = esp32_ble_tracker::ESPBTUUID::from_raw(ble_service);
  }
  void set_ble_char_tx(const char *ble_char_tx) {
    this->ble_char_tx_ = esp32_ble_tracker::ESPBTUUID::from_raw(ble_char_tx);
  }
  void set_ble_char_rx(const char *ble_char_rx) {
    this->ble_char_rx_ = esp32_ble_tracker::ESPBTUUID::from_raw(ble_char_rx);
  }
  void set_ble_encryption(esp_ble_sec_act_t ble_sec_act) { this->ble_sec_act_ = ble_sec_act; }

  virtual bool ble_reg_for_notify() const { return true; }

  bool is_connected() const { return this->node_state == esp32_ble_tracker::ClientState::ESTABLISHED; }
  void connect() { this->parent()->set_enabled(true); }
  void disconnect() { this->parent()->set_enabled(false); }

  void set_disable_scan(bool disable_scan) { this->disable_scan_ = disable_scan; }

 protected:
  uint16_t char_rx_{};
  uint16_t char_tx_{};
  esp32_ble_tracker::ESPBTUUID ble_service_{};
  esp32_ble_tracker::ESPBTUUID ble_char_tx_{};
  esp32_ble_tracker::ESPBTUUID ble_char_rx_{};
  esp_ble_sec_act_t ble_sec_act_{esp_ble_sec_act_t::ESP_BLE_SEC_ENCRYPT_MITM};

  bool disable_scan_{};
};

/// interface io_t {
///   using on_frame_type = etl::delegate<void(const frame_spec_t &frame, size_t size)>;
///   void set_on_frame(on_frame_type &&reader);
///   using on_ready_type = etl::delegate<void()>;
///   void set_on_ready(on_ready_type &&reader);
///   void write(const uint8_t *data, size_t size);
/// };
template<class io_t, class frame_spec_t, class component_t>
class VPortBLEComponentImpl : public VPortIO<io_t, frame_spec_t>, public component_t {
  static_assert(std::is_base_of<Component, component_t>::value, "component_t must derived from Component class");

  using super_t = VPortIO<io_t, frame_spec_t>;

  static constexpr const char *SCHEDULER_NAME = "scheduler";
  static constexpr uint32_t SCHEDULER_TIMEOUT = 3000;

 public:
  VPortBLEComponentImpl(io_t *io) : super_t(io) {
    using this_t = typename std::remove_pointer<decltype(this)>::type;
    this->io_->set_on_frame(io_t::on_frame_type::template create<this_t, &this_t::on_frame_>(*this));
    this->io_->set_on_ready(io_t::on_ready_type::template create<this_t, &this_t::on_ready_>(*this));
  }

  void set_persistent_connection(bool persistent_connection) { this->persistent_connection_ = persistent_connection; }
  bool is_persistent_connection() const { return this->persistent_connection_; }

  using io_type = io_t;
  using frame_spec_type = frame_spec_t;

 protected:
  bool persistent_connection_{};

  void on_ready_() {
    this->fire_ready();
    this->q_free_connection_();
  }

  void on_frame_(const frame_spec_t &frame, size_t size) {
    this->fire_frame(frame, size);
    this->q_free_connection_();
  }

  void q_free_connection_() {
    if (this->is_persistent_connection()) {
      return;
    }
    if (!this->io_->is_connected()) {
      return;
    }
    this->set_timeout(SCHEDULER_NAME, SCHEDULER_TIMEOUT, [this]() { this->io_->disconnect(); });
  }

  bool q_make_connection_() {
    if (!this->io_->is_connected()) {
      this->io_->connect();
      return false;
    }
    if (!this->is_persistent_connection()) {
      this->cancel_timeout(SCHEDULER_NAME);
    }
    return true;
  }
};

template<class io_t, class frame_spec_t, class component_t = Component>
using VPortBLEComponent = VPortQComponent<VPortBLEComponentImpl<io_t, frame_spec_t, component_t>>;

}  // namespace vport
}  // namespace esphome
#endif
