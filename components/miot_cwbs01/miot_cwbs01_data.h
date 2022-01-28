#pragma once

#include "esphome/components/time/real_time_clock.h"

namespace esphome {
namespace miot_cwbs01 {

#pragma pack(push, 1)

enum Packet : uint8_t {
  PACKET_COMMAND_INIT = 0x72,
  PACKET_COMMAND_TIME = 0x05,
  PACKET_COMMAND_MODE = 0x06,
  PACKET_STATE = 0x07,
};

#define COMMAND(command, packet) static_cast<Command>((static_cast<uint16_t>(command) << 8) | packet)

enum Command : uint16_t {
  // enable PACKET_STATE reporting
  COMMAND_ENABLE_STATE_REPORTING = COMMAND(0x00, PACKET_COMMAND_INIT),
  // set power
  COMMAND_POWER /*            */ = COMMAND(0x01, PACKET_COMMAND_MODE),
  // set mode
  COMMAND_MODE /*             */ = COMMAND(0x02, PACKET_COMMAND_MODE),
  // set scene
  COMMAND_SCENE /*            */ = COMMAND(0x03, PACKET_COMMAND_MODE),
  // set schedule
  COMMAND_SCHEDULE /*         */ = COMMAND(0x04, PACKET_COMMAND_MODE),
  // request to sync time
  COMMAND_SYNC_TIME /*        */ = COMMAND(0x05, PACKET_COMMAND_MODE),
  // set cycle
  COMMAND_CYCLE /*            */ = COMMAND(0x06, PACKET_COMMAND_MODE),
  // set time
  COMMAND_SET_TIME /*         */ = COMMAND(0x06, PACKET_COMMAND_TIME),
};

// Scene settings.
enum Scene : uint8_t {
  // Mini refrigirator (smaller than 150L).
  SCENE_MINI_REFRIGIRATOR = 2,
  // Middle refrigirator (150L - 300L)
  SCENE_MIDDLE_REFRIGIRATOR = 3,
  // Big refrigirator
  SCENE_BIG_REFRIGIRATOR = 4,
  // 5 - seater car
  SCENE_FIVE_SEATER_CAR = 5,
  // 7 - seater car
  SCENE_SEVEN_SEATER_CAR = 6,
  // Pet House
  SCENE_PET_HOUSE = 7,
  // Toilet / Bathroom
  SCENE_BATHROOM = 8,
  // Cabinet (shoe cabinet, wardrobe, cabinet)
  SCENE_CABINET = 9,
};

// Mode settings.
enum Mode : uint8_t {
  MODE_NONE = 0,
  // Keep - Freshing.
  MODE_KEEP_FRESHING = 1,
  // Anion.
  MODE_ANION = 2,
  // Deodorization.
  MODE_DEODORIZATION = 3,
  // Purification.
  MODE_PURIFICATION = 4,
  // Deep Purification
  MODE_DEEP_PURIFICATION = 5,
};

enum State : uint8_t {
  STATE_OFF = 0,
  STATE_ON = 1,
};

enum Cycle : uint8_t {
  CYCLE_OFF = 1,
  CYCLE_ON = 2,
};

enum PowerState /*: uint8_t*/ {
  POWER_STATE_BATTERY = 0,
  POWER_STATE_CHARGING = 1,
  POWER_STATE_DISCONNECTED = 2,
};

struct schedule_t {
  // Starting time.
  uint8_t hour_start;
  // Closing time.
  uint8_t hour_close;
  struct {
    // Timer switches.
    bool enabled : 1;
    // Repeat Monday.
    bool mon : 1;
    // Repeat Tuesday.
    bool tue : 1;
    // Repeat Wednesday.
    bool wed : 1;
    // Repeat Thursday.
    bool thu : 1;
    // Repeat Friday.
    bool fri : 1;
    // Repeat Saturday.
    bool sat : 1;
    // Repeat Sunday.
    bool sun : 1;
  } repeat;
};

struct state_t {
  uint8_t /*State*/ state : 4;             // state of device
  uint8_t /*State*/ power : 4;             // state of power button
  uint8_t /*Cycle*/ cycle : 4;             // cycle
  uint8_t /*Mode*/ mode : 4;               // mode
  Scene scene;                             // scene
  uint8_t error : 4;                       // 0 - no error, other value unknown
  uint8_t /*PowerState*/ power_state : 4;  // state of power cable
  uint8_t battery;                         // battery level
  struct : schedule_t {
    uint8_t /*Mode*/ mode : 4;    // mode
    uint8_t /*Scene*/ scene : 4;  // scene
  } schedule;                     // schedule
};

struct datetime_sync_t {
  enum : uint8_t { DO_SYNC = 1 } sync;
};

struct datetime_t {
  uint8_t unknown;  // always 6
  uint8_t year1;    // First 2 digits of year (tm.year / 100).
  uint8_t year2;    // Last 2 digits of year (tm.year % 100).
  uint8_t month;    // 1-12
  uint8_t date;     // 1-31
  uint8_t hour;     // 0-23
  uint8_t minute;   // 0-59
  uint8_t second;   // 0-59
  uint8_t day;      // 1-7, starting from Monday (tm.day_of_week - 1 == 0 ? 7 : tm.day_of_week - 1).
};

#pragma pack(pop)

}  // namespace miot_cwbs01
}  // namespace esphome
