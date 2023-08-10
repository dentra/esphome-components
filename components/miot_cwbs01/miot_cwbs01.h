#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/select/select.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/time/real_time_clock.h"

#include "../miot_client/miot_client.h"

#include "miot_cwbs01_api.h"

namespace esphome {
namespace miot_cwbs01 {

enum UpdateState : uint32_t {
  UPDATE_STATE_POWER = 1 << 0,
  UPDATE_STATE_CYCLE = 1 << 1,
  UPDATE_STATE_MODE = 1 << 2,
  UPDATE_STATE_SCENE = 1 << 3,
};

/**
 * @brief EraClean Refrigerator Odor Eliminator Max.
 */
class MiotCWBS01 : public miot_client::MiotClient,
                   public miot_client::AuthClientListener,
                   public MiotCWBS01Api,
                   public MiotCWBS01ApiListener,
                   //  public esp32_ble_tracker::ESPBTDeviceListener,
                   public binary_sensor::BinarySensor,
                   public PollingComponent {
 public:
  MiotCWBS01() { MiotCWBS01Api::add_listener(this); }

  float get_setup_priority() const override { return setup_priority::DATA; }

  void update() override { this->parent_->set_enabled(true); }

  // bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override {
  //   return device.address_uint64() == this->address_;
  // }

  void on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) override;
  void on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) override;
  void on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) override;

  void on_auth_complete() override;

  void read(const state_t &state) override;
  void read(const datetime_sync_t &dts) override;
  bool send_frame(const void *data, uint8_t size) const override {
    return this->write_char(this->char_.stdio_tx, data, size);
  }

  void set_time(time::RealTimeClock *rtc) { this->rtc_ = rtc; }

  void set_version(text_sensor::TextSensor *version) { this->version_ = version; }
  void set_power(switch_::Switch *power) { this->power_ = power; }
  void set_cycle(switch_::Switch *cycle) { this->cycle_ = cycle; }
  void set_mode(select::Select *mode) { this->mode_ = mode; }
  void set_scene(select::Select *scene) { this->scene_ = scene; }
  void set_battery_level(sensor::Sensor *battery_level) { this->battery_level_ = battery_level; }
  void set_charging(binary_sensor::BinarySensor *charging) { this->charging_ = charging; }
  void set_error(binary_sensor::BinarySensor *error) { this->error_ = error; }

  void set_dirty(uint32_t flag);

 protected:
  struct {
    uint16_t version;
    uint16_t stdio_rx;
    uint16_t stdio_tx;
  } char_{};

  uint32_t update_state_{};

  time::RealTimeClock *rtc_{};
  text_sensor::TextSensor *version_{};
  switch_::Switch *power_{};
  switch_::Switch *cycle_{};
  select::Select *mode_{};
  select::Select *scene_{};
  sensor::Sensor *battery_level_{};
  binary_sensor::BinarySensor *charging_{};
  binary_sensor::BinarySensor *error_{};

  void sync_state_();
  bool is_dirty_() const { return this->update_state_ != 0; }
};

class MiotCWBS01PowerSwitch : public switch_::Switch {
 public:
  explicit MiotCWBS01PowerSwitch(MiotCWBS01 *parent) : parent_(parent) {}

  void write_state(bool state) override {
    this->publish_state(state);
    this->parent_->set_dirty(UPDATE_STATE_POWER);
  }

 protected:
  MiotCWBS01 *parent_;
};

class MiotCWBS01CycleSwitch : public switch_::Switch {
 public:
  explicit MiotCWBS01CycleSwitch(MiotCWBS01 *parent) : parent_(parent) {}

  void write_state(bool state) override {
    this->publish_state(state);
    this->parent_->set_dirty(UPDATE_STATE_CYCLE);
  }

 protected:
  MiotCWBS01 *parent_;
};

class MiotCWBS01ModeSelect : public select::Select {
 public:
  explicit MiotCWBS01ModeSelect(MiotCWBS01 *parent) : parent_(parent) {}
  void control(const std::string &value) override {
    this->publish_state(value);
    this->parent_->set_dirty(UPDATE_STATE_MODE);
  }

 protected:
  MiotCWBS01 *parent_;
};

class MiotCWBS01SceneSelect : public select::Select {
 public:
  explicit MiotCWBS01SceneSelect(MiotCWBS01 *parent) : parent_(parent) {}
  void control(const std::string &value) override {
    this->publish_state(value);
    this->parent_->set_dirty(UPDATE_STATE_SCENE);
  }

 protected:
  MiotCWBS01 *parent_;
};

}  // namespace miot_cwbs01
}  // namespace esphome
