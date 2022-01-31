#pragma once

#include <functional>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"

#include "miot_spec.h"
#include "miot_object.h"
#include "miot_utils.h"

namespace esphome {
namespace miot {

using bindkey_t = uint8_t[16];

struct MiBeacon {
  MiBeacon() : frame_control({}), product_id(0), frame_counter(0){};
  MiBeacon(const RawMiBeaconHeader *hdr)
      : frame_control(hdr->frame_control), product_id(hdr->product_id), frame_counter(hdr->frame_counter) {}
  FrameControl frame_control;
  uint16_t product_id;
  uint8_t frame_counter;
  uint64_t mac_address = {};
  Capability capability = {};
  IOCapability io_capability = {};
  BLEObject object = {};
  uint32_t random_number = {};
  uint32_t message_integrity_check = {};
  bool has_object() const { return frame_control.object_include; }
  bool has_address() const { return frame_control.mac_include; }
  bool has_capability() const { return frame_control.capability_include; }
  bool has_io_capability() const { return frame_control.capability_include && capability.io; }
  bool is_mesh() const { return frame_control.mesh; }
  bool is_registered() const { return frame_control.registered; }
};

class MiotListener;

class MiBeaconTracker : public esp32_ble_tracker::ESPBTDeviceListener, public Component {
 public:
  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;
  void register_listener(MiotListener *listener) { this->listeners_.push_back(listener); }

 protected:
  std::vector<MiotListener *> listeners_ = {};

  bool parse_mibeacon_(const esp32_ble_tracker::ESPBTDevice &device, const std::vector<uint8_t> &data) const;
};

class MiotListener {
 public:
  const uint64_t &get_address() const { return this->address_; }
  void set_address(uint64_t address) { this->address_ = address; };

  const uint8_t *get_bindkey() const { return this->bindkey_; }
  void set_bindkey(const bindkey_t bindkey) { memcpy(this->bindkey_, bindkey, sizeof(bindkey_t)); }
  bool have_bindkey() const;

  virtual uint16_t get_product_id() const = 0;

  virtual bool process_mibeacon(const MiBeacon &mib);

 protected:
  uint64_t address_ = {};
  bindkey_t bindkey_ = {};
  uint8_t frame_counter_ = {};

  bool process_unhandled_(const miot::BLEObject &obj);
  virtual bool process_object_(const BLEObject &obj) = 0;
};

class MiotComponent : public Component, public MiotListener {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }

  virtual const char *get_product_code() const = 0;
  void set_battery_level(sensor::Sensor *battery_level) { this->battery_level_ = battery_level; }
  void set_battery_voltage(sensor::Sensor *battery_voltage) { this->battery_voltage_ = battery_voltage; }

 protected:
  sensor::Sensor *battery_level_{};
  sensor::Sensor *battery_voltage_{};

  void dump_config_(const char *TAG) const;

  bool process_default_(const miot::BLEObject &obj);
};

}  // namespace miot
}  // namespace esphome
