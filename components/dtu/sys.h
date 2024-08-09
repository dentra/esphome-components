#pragma once

#include <string>

namespace esphome {
namespace dtu {

/// Get the last 6 symbols of MAC-address prependend with "-".
std::string get_mac_suffix();

/// Calculate a FNV-1 hash of \p str and return it as string repr.
std::string fnv1_hash_str(const std::string &str);

}  // namespace dtu
}  // namespace esphome
