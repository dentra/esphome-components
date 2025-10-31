#pragma once

#include <cinttypes>
#include <array>
#include <vector>

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "vport.h"

#ifndef USE_VPORT_COMMAND_QUEUE_SIZE
#define USE_VPORT_COMMAND_QUEUE_SIZE 16
#endif

namespace esphome {
namespace vport {

// interface impl_t {
//   using io_type = io_t;
//   using frame_spec_type = frame_spec_t;
//   // free connection.
//   void q_free_connection_();
//   // make a new connection. return true when connection already estabilished.
//   bool q_make_connection_();
// }
template<class impl_t> class VPortQComponent : public impl_t {
  using io_t = typename impl_t::io_type;
  using frame_spec_t = typename impl_t::frame_spec_type;

  static constexpr const char *COMMAND_INTERVAL_NAME = "command";

  template<size_t N> class QRingBuffer {
   public:
    void push(const uint8_t *data, size_t size) {
      if (this->size_ == N) {
        this->head_ = (this->head_ + 1) % N;
        this->size_--;
      }

      this->data_[this->tail_].assign(data, data + size);

      this->tail_ = (this->tail_ + 1) % N;
      this->size_++;
    }

    const std::vector<uint8_t> &front() const { return this->data_[this->head_]; }

    void pop() {
      if (!empty()) {
        this->head_ = (this->head_ + 1) % N;
        this->size_--;
      }
    }

    bool empty() const { return this->size_ == 0; }
    size_t size() const { return this->size_; }
    size_t capacity() const { return N; }

   protected:
    std::array<std::vector<uint8_t>, N> data_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
  };

 public:
  VPortQComponent(io_t *io) : impl_t(io) {}

  void set_command_interval(uint32_t command_interval) { this->command_interval_ = command_interval; }

  void write(const frame_spec_t &frame, size_t size) override {
    const auto *data = reinterpret_cast<const uint8_t *>(&frame);
    this->awaited_.push(data, size);
  }

  void call_setup() override {
    impl_t::call_setup();
    if (this->command_interval_ > 0) {
      this->set_interval(COMMAND_INTERVAL_NAME, this->command_interval_, [this]() { this->q_process_(); });
    }
  }

  void call_loop() override {
    impl_t::call_loop();
    if (this->command_interval_ == 0) {
      this->q_process_();
    }
  }

 protected:
  uint32_t command_interval_{};
  QRingBuffer<USE_VPORT_COMMAND_QUEUE_SIZE> awaited_;

  void q_process_() {
    if (this->awaited_.empty()) {
      impl_t::q_free_connection_();
      return;
    }

    if (!impl_t::q_make_connection_()) {
      return;
    }

    ESP_LOGV("vport_queue", "Processing: %zu of max %zu", this->awaited_.size(), this->awaited_.capacity());

    const auto &el = this->awaited_.front();
    const auto *frame = reinterpret_cast<const frame_spec_t *>(el.data());
    impl_t::write(*frame, el.size());
    this->awaited_.pop();
  }
};

}  // namespace vport
}  // namespace esphome
