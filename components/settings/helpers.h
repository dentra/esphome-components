#pragma once

#include "esphome/core/helpers.h"

namespace esphome {
/// Helper to convers bool to string.
inline std::string to_string(bool value) { return value ? "true" : "false"; }

/// Parse a decimal double floating-point number from a null-terminated string.
template<typename T, enable_if_t<(std::is_same<T, double>::value), int> = 0> optional<T> parse_number(const char *str) {
  char *end = nullptr;
  const double value = ::strtod(str, &end);
  if (end == str || *end != '\0' || value == HUGE_VAL) {
    return {};
  }
  return value;
}

namespace settings {
/// Helper to escape json string.
std::string json_escape(const std::string &str);
/// Helper to sanitize string, clearing all except [0-9A-Za-z].
std::string hex_sanitize(const std::string &str);

/// Helper to parse mac address from string.
bool parse_mac(const std::string& mac, uint64_t *result);
/// Helper to convert mac address to string
std::string mac_to_string(uint64_t mac);
/// Helper to convert mac address to json string.
inline std::string mac_to_json_string(const uint64_t mac) { return "\"" + mac_to_string(mac) + "\""; }

}  // namespace settings
}  // namespace esphome
