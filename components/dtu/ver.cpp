#include "ver.h"

namespace esphome {
namespace dtu {

int parse_version(const char *version) {
  char *next{};
  uint32_t res = std::strtoul(version, &next, 10) * 100000;
  if (next && *next != 0) {
    uint32_t part = std::strtoul(++next, &next, 10);
    if (part < 99) {
      res += part * 1000;
    } else {
      res += 99000;
    }
    if (next && *next != 0) {
      part = std::strtoul(++next, &next, 10);
      if (part < 999) {
        res += part;
      } else {
        res += 999;
      }
    }
  }
  return res;
}

}  // namespace dtu
}  // namespace esphome
