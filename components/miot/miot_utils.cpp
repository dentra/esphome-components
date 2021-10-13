#include "miot_utils.h"

namespace esphome {
namespace miot {

constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

std::string hexstr(const uint8_t *data, uint32_t size) {
  std::string str;
  for (int i = 0; i < size; ++i) {
    str[2 * i + 0] = hexmap[(data[i] & 0xF0) >> 4];
    str[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return str;
}

}  // namespace miot
}  // namespace esphome
