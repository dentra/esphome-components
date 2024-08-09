#include <cinttypes>

#include "esphome/core/helpers.h"

#include "sys.h"

namespace esphome {
namespace dtu {

std::string get_mac_suffix() {
  uint8_t mac[6];
  get_mac_address_raw(mac);
  auto s = format_hex(mac + 3, 3);
  s.insert(s.cbegin(), '-');
  return s;
}

std::string fnv1_hash_str(const std::string &str) { return str_snprintf("%08" PRIx32, 9, fnv1_hash(str)); }

}  // namespace dtu
}  // namespace esphome
