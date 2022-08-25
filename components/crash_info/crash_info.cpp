#include "user_interface.h"
#include "esphome/core/log.h"
#ifdef USE_TIME
#include <ctime>
#include "esphome/components/time/real_time_clock.h"
#endif
#include "crash_info.h"

/*
Some info for possibly future IDF support
https://github.com/espressif/esp-idf/issues/5163#issuecomment-851677154
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/fatal-errors.html
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/core_dump.html
*/

// maximum number of stack frames saved to RTC.
#ifndef DENTRA_CRASH_INFO_MAX_STACK_FRAMES_SIZE
#define DENTRA_CRASH_INFO_MAX_STACK_FRAMES_SIZE 10
#endif
// minimum stack frame address.
#ifndef DENTRA_CRASH_INFO_MIN_STACK_FRAMES_ADDR
#define DENTRA_CRASH_INFO_MIN_STACK_FRAMES_ADDR 0x40000000
#endif
// maximum stack frame address.
#ifndef DENTRA_CRASH_INFO_MAX_STACK_FRAMES_ADDR
#define DENTRA_CRASH_INFO_MAX_STACK_FRAMES_ADDR 0x50000000
#endif

namespace esphome {
namespace crash_info {

static const char *const TAG = "crash_info";

struct crash_info_t {
  uint8_t reason;
  uint8_t exccause;
  uint32_t stack_frames[DENTRA_CRASH_INFO_MAX_STACK_FRAMES_SIZE];
#ifdef USE_TIME
  time_t time;
#endif
} PACKED;

CrashInfo *g_crash_info = {};

extern "C" void custom_crash_callback(rst_info *rst_nfo, uint32_t stack, uint32_t stack_end) {
  if (g_crash_info) {
    auto stack_ptr = reinterpret_cast<const uint32_t *>(stack);
    auto stack_len = (stack_end - stack) / sizeof(uint32_t);
    g_crash_info->save_crash_info(rst_nfo->reason, rst_nfo->exccause, stack_ptr, stack_len);
  }
}

CrashInfo::CrashInfo() { g_crash_info = this; }

uint32_t CrashInfo::get_max_stack_frames_size() const { return DENTRA_CRASH_INFO_MAX_STACK_FRAMES_SIZE; }
uint32_t CrashInfo::get_min_stack_frames_addr() const { return DENTRA_CRASH_INFO_MIN_STACK_FRAMES_ADDR; }
uint32_t CrashInfo::get_max_stack_frames_addr() const { return DENTRA_CRASH_INFO_MAX_STACK_FRAMES_ADDR; }

void CrashInfo::setup() { this->rtc_ = global_preferences->make_preference<crash_info_t>(fnv1_hash(TAG), false); }

void CrashInfo::dump_config() {
  crash_info_t ci{};
  if (!this->rtc_.load(&ci) && ci.reason != REASON_EXCEPTION_RST) {
    if (this->indicator_) {
      this->indicator_->publish_state(false);
    }
    return;
  }

  if (this->indicator_) {
    this->indicator_->publish_state(true);
  }

  ESP_LOGI(TAG, "Crash info: ");
#ifdef USE_TIME
  ESP_LOGI(TAG, "  Exception cause code: %d, time: %s", ci.exccause,
           time::ESPTime::from_epoch_local(ci.time).strftime("%F %T").c_str());
#else
  ESP_LOGI(TAG, "  Exception cause code: %d", ci.exccause);
#endif
  ESP_LOGI(TAG, "  Stacktrace:");
  ESP_LOGV(TAG, "  Max stack frames size: %u", this->get_max_stack_frames_size());
  ESP_LOGV(TAG, "  Min stack frames addr: 0x%08x", this->get_min_stack_frames_addr());
  ESP_LOGV(TAG, "  Max stack frames addr: 0x%08x", this->get_max_stack_frames_addr());
  ESP_LOGV(TAG, "  %s", format_hex(reinterpret_cast<const uint8_t *>(&ci), sizeof(ci)).c_str());
  std::string s;
  s.reserve(10);
  for (int i = 0; i < this->get_max_stack_frames_size(); i++) {
    if (ci.stack_frames[i] == 0) {
      break;
    }
    s.clear();
    for (int j = 0; j < 4 && i < this->get_max_stack_frames_size(); j++, i++) {
      if (ci.stack_frames[i] == 0) {
        break;
      }
      s += str_snprintf(" %08x", 9, ci.stack_frames[i]);
    }
    ESP_LOGI(TAG, "    %s", s.c_str());
  }
}

void CrashInfo::save_crash_info(uint8_t reason, uint8_t exccause, const uint32_t *stack, size_t stack_size) {
  crash_info_t ci{.reason = reason,
                  .exccause = exccause,
                  .stack_frames = {},
#ifdef USE_TIME
                  .time = std::time(nullptr)
#endif
  };

  for (uint32_t i = 0, cnt = 0; i < stack_size && cnt < this->get_max_stack_frames_size(); i++) {
    auto frame = stack[i];
    // try to store only decodable stack frames
    if (frame >= this->get_min_stack_frames_addr() && frame < this->get_max_stack_frames_addr()) {
      ci.stack_frames[cnt++] = frame;
    }
  }
  this->rtc_.save(&ci);
}

void CrashInfo::reset() {
  crash_info_t ci{};
  this->rtc_.save(&ci);
  if (this->indicator_) {
    this->indicator_->publish_state(false);
  }
}

}  // namespace crash_info
}  // namespace esphome
