#pragma once

#include "../qingping_pc.h"

namespace esphome {
namespace qingping {
namespace property_controller {

struct Temperature : ValueHolder<qingping::QPDataTH> {
  float state() const { return this->value->get_temperature(); }
};

struct Humidity : ValueHolder<qingping::QPDataTH> {
  float state() const { return this->value->get_humidity(); }
};

struct BatteryLevel : ValueHolder<qingping::QPDataBatteryLevel> {
  float state() const { return this->value->get_battery_level(); }
};

struct Pressure : ValueHolder<qingping::QPDataPressure> {
  float state() const { return this->value->get_pressure(); }
};

struct Illuminance : RawValueHolder {
  float state() const {
    auto *illuminance = this->cast<qingping::QPDataIlluminance>();
    if (illuminance) {
      return illuminance->get_illuminance();
    }
    auto *motion_illuminance = this->cast<qingping::QPDataMotionIlluminance>();
    if (motion_illuminance) {
      return motion_illuminance->get_illuminance();
    }
    return NAN;
  }
};

struct Packet : ValueHolder<qingping::QPDataPacket> {
  float state() const { return this->value->raw_packet; }
};

struct PM25 : ValueHolder<qingping::QPDataPM> {
  float state() const { return this->value->get_pm25(); }
};

struct PM10 : ValueHolder<qingping::QPDataPM> {
  float state() const { return this->value->get_pm10(); }
};

struct CO2 : ValueHolder<qingping::QPDataCO2> {
  float state() const { return this->value->get_co2(); }
};

}  // namespace property_controller
}  // namespace qingping
}  // namespace esphome
