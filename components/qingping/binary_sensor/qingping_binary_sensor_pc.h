#pragma once

#include "../qingping_pc.h"

namespace esphome {
namespace qingping {
namespace property_controller {

struct Door : ValueHolder<qingping::QPDataDoorState> {
  bool state() const { return this->value->get_door_state(); }
};

struct DoorProblem : ValueHolder<qingping::QPDataDoorState> {
  bool state() const { return this->value->get_door_left_open_problem(); }
};

struct Motion : ValueHolder<qingping::QPDataMotionIlluminance> {
  bool state() const { return this->value->get_motion_state(); }
};

struct Light : ValueHolder<qingping::QPDataLightState> {
  bool state() const { return this->value->get_light_state(); }
};

}  // namespace property_controller
}  // namespace qingping_binaray_sensor
}  // namespace esphome
