#pragma once

#include <algorithm>
#include <cstring>
#include <string>

namespace esphome {
namespace settings {

// Node name restored from NVS lives in a static buffer owned by settings.
static constexpr size_t NODE_NAME_BUF_SIZE = 100;
// 7 bytes are reserved for the "-XXXXXX" MAC suffix placeholder.
static constexpr size_t NODE_NAME_MAX_LEN = NODE_NAME_BUF_SIZE - 8;

// Copies the stored node name into buf, appends the MAC suffix placeholder when requested
// and always terminates the result. The name may come from the settings UI, which limits
// its length only in the browser, so it is clamped here. Returns the length to pass to
// App.pre_setup().
inline size_t restore_node_name(char *buf, const std::string &name, bool add_mac_suffix) {
  size_t len = std::min(name.size(), NODE_NAME_MAX_LEN);
  memcpy(buf, name.c_str(), len);
  if (add_mac_suffix) {
    buf[len] = '-';
    memcpy(buf + len + 1, "XXXXXX", 6);
    len += 7;
  }
  buf[len] = 0;
  return len;
}

}  // namespace settings
}  // namespace esphome
