#pragma once

#include "miot_cwbs01_data.h"

namespace esphome {
namespace miot_cwbs01 {

class MiotCWBS01ApiListener {
 public:
  virtual void read(const state_t &state) = 0;
  virtual void read(const datetime_sync_t &dts) = 0;
};

class MiotCWBS01Api {
 public:
  void add_listener(MiotCWBS01ApiListener *listener) { listeners_.push_back(listener); }

  bool read_frame(const uint8_t *data, uint8_t size);
  virtual bool send_frame(const void *data, uint8_t size) const = 0;

  bool request_state() const;
  bool sync_time(const esphome::time::ESPTime &tm) const;
  bool set_cycle(bool enabled) const;
  bool set_power(bool enabled) const;
  bool set_mode(Mode mode) const;
  bool set_scene(Scene scene) const;
  bool set_schedule(const schedule_t &schedule, Scene scene, Mode mode) const;

 protected:
  std::vector<MiotCWBS01ApiListener *> listeners_;

  bool read_command_(Command cmd, const void *data, uint8_t size);
  bool send_command_(Command cmd, const void *data, uint8_t size) const;
  bool read_packet_(Packet type, const void *data, uint8_t size);
  bool send_packet_(Packet type, const void *data, uint8_t size) const;
};

}  // namespace miot_cwbs01
}  // namespace esphome
