#pragma once

#include <cmath>
#include <type_traits>

#include "qingping_beacon.h"

namespace esphome {
namespace qingping {
namespace property_controller {

template<class T> struct ValueHolder {
  using data_type = T;
  const T *value;
};

struct RawValueHolder : ValueHolder<QPDataRaw> {
  const QPID type;
  template<class T> const T *cast() const {
    if (T::type != this->type) {
      return nullptr;
    }
    return reinterpret_cast<const T *>(this->value);
  }
};

template<class T> struct BinaryState : T {
  bool valid() const { return T::value != nullptr; }
  bool state() const { return T::state(); }
};

template<class T> struct State : T {
  bool valid() const { return T::value != nullptr; }
  float state() const { return T::state(); }
};

template<class T> BinaryState<T> make_binary_state(const qingping::QPDataPoint &dp) {
  const auto *value = dp.cast<typename T::data_type>();
  return BinaryState<T>{value};
}

template<class T> State<T> make_state(const qingping::QPDataPoint &dp) {
  const auto *value = dp.cast<typename T::data_type>();
  if constexpr (std::is_base_of<T, RawValueHolder>::value) {
    return State<T>{value, dp.type};
  } else {
    return State<T>{value};
  }
}

}  // namespace property_controller
}  // namespace qingping
}  // namespace esphome
