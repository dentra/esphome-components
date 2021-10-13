#pragma once

#include "inttypes.h"

namespace esphome {
namespace miot {

inline const uint8_t *mac_reverse(const uint64_t &mac) { return reinterpret_cast<const uint8_t *>(&mac); }

}  // namespace miot
}  // namespace esphome
