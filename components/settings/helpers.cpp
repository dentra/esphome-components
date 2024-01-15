#include "helpers.h"

namespace esphome {
namespace settings {

std::string json_escape(const std::string &str) {
  std::string out;
  out.reserve(str.length());
  for (const auto c : str) {
    switch (c) {
      case '"':
        out.append("\\\"");
        break;
      case '\\':
        out.append("\\\\");
        break;
      case '\b':
        out.append("\\b");
        break;
      case '\f':
        out.append("\\f");
        break;
      case '\n':
        out.append("\\n");
        break;
      case '\r':
        out.append("\\r");
        break;
      case '\t':
        out.append("\\t");
        break;
      default: {
        if (c < ' ') {
          out.append(str_snprintf("\\u%04x", 6, static_cast<int>(c)));
        } else {
          out.push_back(c);
        }
      }
    }
  }
  return out;
}

std::string hex_sanitize(const std::string &str) {
  std::string out;
  out.reserve(str.length());
  std::copy_if(str.begin(), str.end(), std::back_inserter(out), [](const char c) -> bool {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
  });
  return out;
}

bool parse_mac(const std::string &mac, uint64_t *result) {
  if (mac.length() < 12) {
    return false;
  }
  auto m = hex_sanitize(mac);
  if (m.length() != 12) {
    return false;
  }
  if (parse_hex(m.c_str(), m.length(), reinterpret_cast<uint8_t *>(result), sizeof(*result)) == 0) {
    return false;
  }
  *result = byteswap(*result) & 0xFFFFFFFFFFFFULL;
  return true;
}

std::string mac_to_string(const uint64_t mac) {
  return str_snprintf("%02X:%02X:%02X:%02X:%02X:%02X", 17, (uint8_t) (mac >> 40) & 0xff, (uint8_t) (mac >> 32) & 0xff,
                      (uint8_t) (mac >> 24) & 0xff, (uint8_t) (mac >> 16) & 0xff, (uint8_t) (mac >> 8) & 0xff,
                      (uint8_t) (mac >> 0) & 0xff);
}

}  // namespace settings
}  // namespace esphome
