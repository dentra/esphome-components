#pragma once
#ifdef ARDUINO_ARCH_ESP32

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/select/select.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/time/real_time_clock.h"

#include "../miot/miot.h"
#include "../miot_client/miot_client.h"

#include "miot_cwbs01_api.h"

namespace esphome {
namespace miot_cwbs01 {

class MiotCWBS01ModeSelect;
class MiotCWBS01SceneSelect;

/**
 * @brief EraClean Refrigerator Odor Eliminator Max.
 */
class MiotCWBS01 : public miot_client::MiotClient,
                   public miot_client::AuthClientListener,
                   public MiotCWBS01Api,
                   public MiotCWBS01ApiListener,
                   public esp32_ble_tracker::ESPBTDeviceListener,
                   public binary_sensor::BinarySensor,
                   public PollingComponent {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }

  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override {
    return device.address_uint64() == this->address_;
  }

  void on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) override;
  void on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) override;
  void on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) override;

  void update() override {}

  void on_auth_complete() override;

  void read(const state_t &state) override;
  void read(const datetime_sync_t &dts) override;
  bool send_frame(const void *data, uint8_t size) const { return this->write_char(this->char_.stdio_tx, data, size); }

  void set_time(time::RealTimeClock *rtc) { this->rtc_ = rtc; }
  void set_version(text_sensor::TextSensor *version) { this->version_ = version; }

  void set_power(switch_::Switch *power) { this->power_ = power; }
  void set_mode(MiotCWBS01ModeSelect *mode) { this->mode_ = mode; }
  void set_cycle(switch_::Switch *cycle) { this->cycle_ = cycle; }
  void set_scene(MiotCWBS01SceneSelect *scene) { this->scene_ = scene; }
  void set_charging(binary_sensor::BinarySensor *charging) { this->charging_ = charging; }
  void set_error(binary_sensor::BinarySensor *error) { this->error_ = error; }
  void set_battery_level(sensor::Sensor *battery_level) { this->battery_level_ = battery_level; }

 protected:
  struct {
    uint16_t version;
    uint16_t stdio_rx;
    uint16_t stdio_tx;
  } char_ = {};

  time::RealTimeClock *rtc_;
  text_sensor::TextSensor *version_{};
  switch_::Switch *power_ = {};
  MiotCWBS01ModeSelect *mode_ = {};
  switch_::Switch *cycle_ = {};
  MiotCWBS01SceneSelect *scene_ = {};
  binary_sensor::BinarySensor *charging_ = {};
  binary_sensor::BinarySensor *error_ = {};
  sensor::Sensor *battery_level_ = {};
};

class MiotCWBS01PowerSwitch : public switch_::Switch {
 public:
  explicit MiotCWBS01PowerSwitch(MiotCWBS01Api *api) : api_(api) {}

 protected:
  MiotCWBS01Api *api_;
  void write_state(bool state) override { this->api_->set_power(state); }
};

class MiotCWBS01CycleSwitch : public switch_::Switch {
 public:
  explicit MiotCWBS01CycleSwitch(MiotCWBS01Api *api) : api_(api) {}

 protected:
  MiotCWBS01Api *api_;
  void write_state(bool state) override { this->api_->set_cycle(state); }
};

template<typename T, int S> class MiotCWBS01Select : public select::Select {
 public:
  explicit MiotCWBS01Select(MiotCWBS01Api *api) : api_(api) {}

  const std::string &get_option(int index) const {
    auto idx = index - S;
    auto &options = this->traits.get_options();
    return options[idx];
  }

  void control(const std::string &value) override {
    auto &options = this->traits.get_options();
    auto pos = std::find(options.begin(), options.end(), value);
    if (pos == options.end()) {
      return;
    }
    auto idx = std::distance(options.begin(), pos) - S;
    this->select(static_cast<T>(idx));
  }

  virtual void select(T value) const = 0;

 protected:
  MiotCWBS01Api *api_;
};

class MiotCWBS01ModeSelect : public MiotCWBS01Select<Mode, Mode::MODE_NONE> {
 public:
  explicit MiotCWBS01ModeSelect(MiotCWBS01Api *api) : MiotCWBS01Select(api) {}
  void select(Mode value) const override { this->api_->set_mode(value); }
};

class MiotCWBS01SceneSelect : public MiotCWBS01Select<Scene, Scene::SCENE_MINI_REFRIGIRATOR> {
 public:
  explicit MiotCWBS01SceneSelect(MiotCWBS01Api *api) : MiotCWBS01Select(api) {}
  void select(Scene value) const override { this->api_->set_scene(value); }
};

}  // namespace miot_cwbs01
}  // namespace esphome

#endif
