#pragma once
#include "esphome/core/defines.h"
#ifdef USE_ESP_IDF
#include <string>

namespace esphome {
namespace dtu {

bool check_url_available(const char *url);
inline bool check_url_available(const std::string &url) { return check_url_available(url.c_str()); }

}  // namespace dtu
}  // namespace esphome
#endif
