#include "esphome/core/log.h"

#include "miot_cwbs01_api.h"

namespace esphome {
namespace miot_cwbs01 {

static const char *const TAG = "miot_cwbs01.api";

// *** TX ***
// AA03 72.00 xx                  request state
// AA0B 05.06 0 0 0 0 0 0 0 0 xx  sync time
// AA04 06.06 0 xx                cycle (OFF=1, ON=2)
// AA04 06.01 0 xx                power (OFF=0, ON=1)
// AA04 06.02 0 xx                mode=[0,1,2,3,4,5])
// AA04 06.03 0 xx                scene=[2,3,4,5,6,7,8,9]
// AA08 06.04 0 0 0 0 0 xx        schedule

// *** RX ***
// request to sync time  aa04 06.05 01 10
// AA04 06.05 01 xx
// report data
// AA0B 07 0 0 0 0 0 0 0 0 0 xx

struct frame_t {
  enum : uint8_t { FRAME_HEADER = 0xAA } header;
  uint8_t size;
  Packet type;
  uint8_t data[sizeof(uint8_t)];  // crc size
} PACKED;

static uint8_t calc_crc(uint8_t init, const void *data, size_t size) {
  auto data8 = static_cast<const uint8_t *>(data);
  while (size--) {
    init += *data8++;
  }
  return init;
}

static uint8_t calc_crc(const frame_t *frame) { return calc_crc(0, &frame->size, frame->size); }

bool MiotCWBS01Api::read_frame(const uint8_t *data, uint8_t size) {
  ESP_LOGV(TAG, "read frame: %s", format_hex_pretty(data, size).c_str());

  auto frame = reinterpret_cast<const frame_t *>(data);
  if (frame->header != frame_t::FRAME_HEADER) {
    ESP_LOGW(TAG, "Invalid frame magic: %s", format_hex_pretty(data, size).c_str());
    return false;
  }

  if (frame->size != size - 1 /*header*/ - 1 /*size*/) {
    ESP_LOGW(TAG, "Invalid frame size: %s", format_hex_pretty(data, size).c_str());
    return false;
  }

  const uint8_t crc = data[size - 1];
  if (calc_crc(frame) != crc) {
    ESP_LOGW(TAG, "Invalid frame crc: %s", format_hex_pretty(data, size).c_str());
    return false;
  }

  return this->read_packet_(frame->type, frame->data, frame->size - 1 /*type*/ - 1 /*crc*/);
}

bool MiotCWBS01Api::read_packet_(Packet type, const void *data, uint8_t size) {
  ESP_LOGV(TAG, "read packet %02x: %s", type, format_hex_pretty(static_cast<const uint8_t *>(data), size).c_str());
  if (type == PACKET_STATE) {
    if (size != sizeof(state_t)) {
      ESP_LOGW(TAG, "Incorrect state packet size: %u", size);
      return false;
    }
    auto state = static_cast<const state_t *>(data);
    ESP_LOGD(TAG, "Got state: %s", format_hex_pretty(reinterpret_cast<uint8_t *>(&state), sizeof(state)).c_str());
    for (auto listener : this->listeners_) {
      listener->read(*state);
    }
    return true;
  }

  if (type == PACKET_COMMAND_MODE) {
    auto data8 = static_cast<const uint8_t *>(data);
    return this->read_command_(COMMAND(*data8, type), data8 + 1, size - 1);
  }

  ESP_LOGW(TAG, "Unknown packet type: %02x, data: %s", type,
           format_hex_pretty(reinterpret_cast<const uint8_t *>(&data), size).c_str());

  return false;
}

bool MiotCWBS01Api::read_command_(Command cmd, const void *data, uint8_t size) {
  if (cmd == COMMAND_SYNC_TIME) {
    if (size != sizeof(datetime_sync_t)) {
      ESP_LOGW(TAG, "Incorrect time sync packet size: %u", size);
      return false;
    }
    auto dt_sync = reinterpret_cast<const datetime_sync_t *>(data);
    ESP_LOGD(TAG, "Got datetime sync: %s",
             format_hex_pretty(reinterpret_cast<uint8_t *>(&dt_sync), sizeof(dt_sync)).c_str());
    for (auto listener : this->listeners_) {
      listener->read(*dt_sync);
    }
    return true;
  }
  return false;
}

bool MiotCWBS01Api::send_packet_(Packet type, const void *data, uint8_t size) const {
  ESP_LOGV(TAG, "send packet %02x, %s", type, format_hex_pretty(static_cast<const uint8_t *>(data), size).c_str());
  auto frame_size = sizeof(frame_t) + size;
  auto frame = static_cast<frame_t *>(std::malloc(frame_size));
  if (frame == nullptr) {
    ESP_LOGE(TAG, "Can't allocate %u bytes", frame_size);
    return false;
  }

  frame->header = frame_t::FRAME_HEADER;
  frame->size = frame_size - sizeof(frame->header) - sizeof(frame->data);
  frame->type = type;
  if (size > 0) {
    std::memcpy(frame->data, data, size);
  }
  frame->data[size] = calc_crc(frame);

  bool res = this->send_frame(frame, frame_size);

  std::free(frame);

  return res;
}

bool MiotCWBS01Api::send_command_(Command cmd, const void *data, uint8_t size) const {
  ESP_LOGV(TAG, "send command %04x, %s", cmd, format_hex_pretty(static_cast<const uint8_t *>(data), size).c_str());
  Packet p = static_cast<Packet>(cmd & 0xFF);
  uint8_t c = cmd >> 8;

  if (size == 0) {
    return this->send_packet_(p, &c, sizeof(c));
  }

  auto pack_size = size + sizeof(c);
  auto pack = static_cast<uint8_t *>(std::malloc(pack_size));
  if (pack == nullptr) {
    ESP_LOGE(TAG, "Can't allocate %u bytes", pack_size);
    return false;
  }
  pack[0] = c;
  std::memcpy(pack + sizeof(c), data, size);
  bool res = this->send_packet_(p, pack, pack_size);
  std::free(pack);

  return res;
}

bool MiotCWBS01Api::request_state() const {
  ESP_LOGD(TAG, "Request state");
  return this->send_command_(COMMAND_REQUEST_STATE, nullptr, 0);
}

bool MiotCWBS01Api::sync_time(const ESPTime &tm) const {
  ESP_LOGD(TAG, "Sync time %04u-%02u-%02u %02u:%02u:%02u", tm.year, tm.month, tm.day_of_month, tm.hour, tm.minute,
           tm.second);
  if (!tm.is_valid()) {
    ESP_LOGW(TAG, "datetime fields out of range");
    return false;
  }
  struct {
    uint8_t year1;   // First 2 digits of year (tm.year / 100).
    uint8_t year2;   // Last 2 digits of year (tm.year % 100).
    uint8_t month;   // 1-12
    uint8_t date;    // 1-31
    uint8_t hour;    // 0-23
    uint8_t minute;  // 0-59
    uint8_t second;  // 0-59
    uint8_t day;     // 1-7, starting from Monday (tm.day_of_week - 1 == 0 ? 7 : tm.day_of_week - 1).
  } data{
      .year1 = static_cast<uint8_t>(tm.year / 100),
      .year2 = static_cast<uint8_t>(tm.year % 100),
      .month = tm.month,
      .date = tm.day_of_month,
      .hour = tm.hour,
      .minute = tm.minute,
      .second = tm.second,
      .day = static_cast<uint8_t>(tm.day_of_week - 1 == 0 ? 7 : tm.day_of_week - 1),
  };
  return this->send_command_(COMMAND_SET_TIME, &data, sizeof(data));
}

bool MiotCWBS01Api::set_cycle(bool enabled) const {
  ESP_LOGD(TAG, "Set cycle %s", ONOFF(enabled));
  struct {
    Cycle cycle;
  } data{enabled ? Cycle::CYCLE_ON : Cycle::CYCLE_OFF};
  return this->send_command_(COMMAND_CYCLE, &data, sizeof(data));
}

bool MiotCWBS01Api::set_power(bool enabled) const {
  ESP_LOGD(TAG, "Set power %s", ONOFF(enabled));
  struct {
    State state;
  } data{enabled ? State::STATE_ON : State::STATE_OFF};
  return this->send_command_(COMMAND_POWER, &data, sizeof(data));
}

bool MiotCWBS01Api::set_mode(Mode mode) const {
  ESP_LOGD(TAG, "Set mode %u", mode);
  struct {
    Mode mode;
  } data{mode};
  return this->send_command_(COMMAND_MODE, &data, sizeof(data));
}

bool MiotCWBS01Api::set_scene(Scene scene) const {
  ESP_LOGD(TAG, "Set scene %u", scene);
  struct {
    Scene scene;
  } data{scene};
  return this->send_command_(COMMAND_SCENE, &data, sizeof(data));
}

bool MiotCWBS01Api::set_schedule(const schedule_t &schedule, Scene scene, Mode mode) const {
  struct {
    schedule_t schedule;
    Scene scene;
    Mode mode;
  } data{.schedule = schedule, .scene = scene, .mode = mode};
  ESP_LOGD(TAG, "Set schedule %s, scene %u, mode %u",
           format_hex_pretty(reinterpret_cast<const uint8_t *>(&data), sizeof(data)).c_str(), scene, mode);
  return this->send_command_(COMMAND_SCHEDULE, &data, sizeof(data));
}
}  // namespace miot_cwbs01
}  // namespace esphome
