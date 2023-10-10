#pragma once

#include <cinttypes>

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/defines.h"

#include "etl/circular_buffer.h"

#include "vport.h"

#ifndef USE_VPORT_COMMAND_QUEUE_SIZE
#define USE_VPORT_COMMAND_QUEUE_SIZE 10
#endif

namespace esphome {
namespace vport {

template<class impl_t> class VPortQComponent : public impl_t {
  using io_t = typename impl_t::io_type;
  using frame_spec_t = typename impl_t::frame_spec_type;

  static constexpr const char *COMMAND_NAME = "command";

 public:
  VPortQComponent(io_t *io) : impl_t(io) {}

  void write(const frame_spec_t &frame, size_t size) override { this->enqueue_(frame, size); }

  void call_setup() override {
    this->setup();
    if (this->command_interval_) {
      this->set_interval(COMMAND_NAME, this->command_interval_, [this]() { this->dequeue_(); });
    }
  }

  void call_loop() override {
    this->loop();
    if (this->command_interval_ == 0) {
      this->dequeue_();
    }
  }

  void set_command_interval(uint32_t interval) { this->command_interval_ = interval; }

 protected:
  uint32_t command_interval_{};
  etl::circular_buffer<std::vector<uint8_t>, USE_VPORT_COMMAND_QUEUE_SIZE> awaited_;

  void enqueue_(const frame_spec_t &frame, size_t size) {
    const auto *data = reinterpret_cast<const uint8_t *>(&frame);
    this->awaited_.push(std::vector<uint8_t>(data, data + size));
  }

  void dequeue_() {
    if (this->awaited_.empty()) {
      impl_t::schedule_disconnect();
      return;
    }

    if (!impl_t::is_connected()) {
      impl_t::connect();
      return;
    }

    impl_t::cancel_disconnect();

    const auto &el = this->awaited_.front();
    const auto *frame = reinterpret_cast<const frame_spec_t *>(el.data());
    impl_t::write(*frame, el.size());
    this->awaited_.pop();
  }
};

}  // namespace vport
}  // namespace esphome
