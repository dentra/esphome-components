#pragma once
#include <string>
#include <inttypes.h>

namespace esphome {
namespace miot {

inline const uint8_t *mac_reverse(const uint64_t &mac) { return reinterpret_cast<const uint8_t *>(&mac); }

std::string hexstr(const uint8_t *data, uint32_t size);
template<typename T> std::string hexstr(const T &data) { return hexstr(data.data(), data.size()); }

}  // namespace miot
}  // namespace esphome
