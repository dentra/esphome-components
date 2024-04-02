#pragma once

#include <cinttypes>
#include <type_traits>

namespace esphome {
namespace qingping {

#pragma pack(push, 1)

enum class QPID : uint8_t {
  TH = 0x01,                  // int16/10 + uint16/10
  BATTERY = 0x02,             // uint8
  DOOR = 0x4,                 // uint8, 0-closed, 1-open, 2-left_open
  PRESSURE = 0x07,            // uint16 / 100
  MOTION_ILLUMINANCE = 0x08,  // uint8 + uint24
  ILLUMINANCE = 0x09,         // uint32
  PACKET = 0x0F,              // uint8
  LIGHT = 0x11,               // bool8
  PM = 0x12,                  // uint16 + uint16
  CO2 = 0x13,                 // uint16
};

struct QPMacAddress {
  union {
    struct {
      uint64_t address : 48;
    };
    uint8_t raw[6];
  };
};

struct QPBeaconHeader {
  // https://github.com/ClearGrass/BleDemo_Android/blob/efa936a6934500488c699f977610b575059610b9/app/qpblue/src/main/java/com/cleargrass/lib/blue/data/ScanResultParsed.kt#L24
  // https://github.com/ClearGrass/BleDemo_iOS/blob/ec9144e7f85a1f9c65d164a5d539b05216de5f7e/Bluetooth/ScanResultParsed.swift#L28
  union {
    struct {
      // bit 0. aes.
      bool aes : 1;
      // bit 1. binding.
      bool binding : 1;
      // bit 2. is booting.
      bool booting : 1;
      // bit 3-5.
      uint8_t version : 3;
      // bit 6. is event.
      bool event : 1;
      // bit 7. has bind.
      bool bind : 1;
    } frame_control;
    uint8_t raw_frame_control;
  };
  uint8_t product_id;
  QPMacAddress mac;
};

struct QPDataPointHeader {
  union {
    uint8_t raw_type;
    QPID type;
  };
  uint8_t size;
};

struct QPDataRaw;

struct QPDataPoint : QPDataPointHeader {
  union {
    uint8_t data0[0];
    uint8_t data1[1];
    uint8_t data2[2];
    uint8_t data4[4];
  };
  template<class T> const T *cast() const {
    if constexpr (!std::is_same<T, QPDataRaw>::value) {
      if (T::type != this->type) {
        return nullptr;
      }
    }
    if (sizeof(T) != this->size) {
      return nullptr;
    }
    return reinterpret_cast<const T *>(this->data0);
  }
};
static_assert(sizeof(QPDataPoint) == 6);

struct QPDataRaw {
  union {
    struct {
      // 1-byte boolean.
      bool b8 : 8;
    };
    // 1-byte unsiged.
    uint8_t u8;
    // 1-byte siged.
    uint8_t i8;
    // 2-byte unsiged.
    uint16_t u16;
    // 2-byte siged.
    int16_t i16;
    struct {
      // 3-byte unsiged.
      uint32_t u24 : 24;
    };
    // 4-byte unsiged.
    uint32_t u32;
    // 4-byte siged.
    int32_t i32;
    // raw bytes.
    uint8_t raw[4];
  };
};
static_assert(sizeof(QPDataRaw) == 4);

struct QPDataTH {
  constexpr static QPID type = QPID::TH;
  uint16_t raw_temperature;
  uint16_t raw_humidity;
  float get_temperature() const { return this->raw_temperature * 0.1; }
  float get_humidity() const { return this->raw_humidity * 0.1; }
};
static_assert(sizeof(QPDataTH) == 4);

struct QPDataBatteryLevel {
  constexpr static QPID type = QPID::BATTERY;
  uint8_t raw_battery_level;
  float get_battery_level() const { return this->raw_battery_level; }
};
static_assert(sizeof(QPDataBatteryLevel) == 1);

struct QPDataDoorState {
  constexpr static QPID type = QPID::DOOR;
  uint8_t raw_door_state;
  bool get_door_state() const { return this->raw_door_state != 0; }
  bool get_door_left_open_problem() const { return this->raw_door_state == 2; }
};
static_assert(sizeof(QPDataDoorState) == 1);

struct QPDataPressure {
  constexpr static QPID type = QPID::PRESSURE;
  uint16_t raw_pressure;
  float get_pressure() const { return this->raw_pressure * 0.1; }
};
static_assert(sizeof(QPDataPressure) == 2);

struct QPDataMotionIlluminance {
  constexpr static QPID type = QPID::MOTION_ILLUMINANCE;
  uint8_t raw_motion;
  struct {
    uint32_t raw_illuminance : 24;
  };
  bool get_motion_state() const { return this->raw_motion != 0; }
  float get_illuminance() const { return this->raw_illuminance; }
};
static_assert(sizeof(QPDataMotionIlluminance) == 4);

struct QPDataIlluminance {
  constexpr static QPID type = QPID::ILLUMINANCE;
  uint32_t raw_illuminance;
  float get_illuminance() const { return this->raw_illuminance; }
};
static_assert(sizeof(QPDataIlluminance) == 4);

struct QPDataPacket {
  constexpr static QPID type = QPID::PACKET;
  uint8_t raw_packet;
};
static_assert(sizeof(QPDataPacket) == 1);

struct QPDataLightState {
  constexpr static QPID type = QPID::LIGHT;
  uint8_t raw_light_state;
  bool get_light_state() const { return this->raw_light_state != 0; }
};
static_assert(sizeof(QPDataLightState) == 1);

struct QPDataPM {
  constexpr static QPID type = QPID::PM;
  uint16_t raw_pm25;
  uint16_t raw_pm10;
  float get_pm25() const { return this->raw_pm25; }
  float get_pm10() const { return this->raw_pm10; }
};
static_assert(sizeof(QPDataPM) == 4);

struct QPDataCO2 {
  constexpr static QPID type = QPID::CO2;
  uint16_t raw_co2;
  float get_co2() const { return this->raw_co2; }
};
static_assert(sizeof(QPDataCO2) == 2);

#pragma pack(pop)

}  // namespace qingping
}  // namespace esphome
