#pragma once
#ifdef USE_VPORT_BLE

#include "etl/circular_buffer.h"

#include "esphome/components/ble_client/ble_client.h"

#include "vport.h"

namespace esphome {
namespace vport {

#define VPORT_BLE_LOG(port_name) \
  VPORT_LOG(port_name); \
  this->io_->dump_settings(TAG); \
  ESP_LOGCONFIG(TAG, "  Persistent connection: %s", ONOFF(this->persistent_connection_));

class VPortBLENode : public ble_client::BLEClientNode {
 public:
  void dump_settings(const char *TAG) const;
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

/// interface BleIO {
///   using on_frame_type = etl::delegate<void(const frame_spec_t &frame, size_t size)>;
///   void set_on_frame(on_frame_type &&reader);
///   using on_ready_type = etl::delegate<void()>;
///   void set_on_ready(on_ready_type &&reader);
///   void write(const uint8_t *data, size_t size);
/// };
template<class io_t, class frame_spec_t, class component_t = PollingComponent>
class VPortBLEComponent : public VPortIO<io_t, frame_spec_t>, public component_t {
  using vportio_t = VPortIO<io_t, frame_spec_t>;

 public:
  VPortBLEComponent(io_t *io) : vportio_t(io) {
    using this_t = typename std::remove_pointer<decltype(this)>::type;
    this->io_->set_on_frame(io_t::on_frame_type::template create<this_t, &this_t::on_frame_>(*this));
    this->io_->set_on_ready(io_t::on_ready_type::template create<this_t, &this_t::on_ready_>(*this));
  }

  void write(const frame_spec_t &frame, size_t size) override { this->enqueue_(frame, size); }

  void set_persistent_connection(bool persistent_connection) { this->persistent_connection_ = persistent_connection; }
  bool is_persistent_connection() const { return this->persistent_connection_; }

  void schedule_disconnect(uint32_t timeout = 3000) {
    if (timeout) {
      this->set_timeout(this->scheduler_name_(), timeout, [this]() { this->io_->disconnect(); });
    }
  }
  void cancel_disconnect() { this->cancel_timeout(this->scheduler_name_()); }

  virtual bool can_disconnect(const frame_spec_t &frame, size_t size) { return true; }

  // FIXME перенесено из tion_ble
  // по идее, здесь вообще не нужен PollingComponent т.к. полинг идет из основного/управляющего компонента
  void update() override {
    if (this->io_->is_connected()) {
      // FIXME полсе соединения была возможность сразу же запросить статус
      // теперь запрос отправляется из компонента чере vport.write
      // this->fire_poll();
      if (!this->is_persistent_connection()) {
        // FIXME нужно понять что делать с state_timeout_
        this->schedule_disconnect(this->state_timeout_);
      }
    } else {
      this->io_->connect();
    }
  }

 protected:
  bool persistent_connection_{};
  uint32_t state_timeout_{};

  const std::string scheduler_name_() const { return this->io_->parent()->address_str(); }

  void on_ready_() {
    this->dequeue_();
    this->fire_ready();
    if (!this->is_persistent_connection()) {
      this->schedule_disconnect();
    }
  }

  void on_frame_(const frame_spec_t &frame, size_t size) {
    this->fire_frame(frame, size);
    if (this->can_disconnect(frame, size)) {
      this->cancel_disconnect();
      if (!this->is_persistent_connection()) {
        this->schedule_disconnect();
      }
    }
  }

  void enqueue_(const frame_spec_t &frame, size_t size) {
    if (this->io_->is_connected()) {
      this->dequeue_();
      vportio_t::write(frame, size);
    } else {
      auto data = reinterpret_cast<const uint8_t *>(&frame);
      this->awaited_.push(std::vector<uint8_t>(data, data + size));
      this->io_->connect();
    }
  }

  void dequeue_() {
    while (!this->awaited_.empty()) {
      const auto &el = this->awaited_.front();
      auto frame = reinterpret_cast<const frame_spec_t *>(el.data());
      vportio_t::write(*frame, el.size());
      this->awaited_.pop();
    }
  }

  etl::circular_buffer<std::vector<uint8_t>, 10> awaited_;
};

}  // namespace vport
}  // namespace esphome
#endif
