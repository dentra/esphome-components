#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/version.h"

#include "esphome/components/web_server_base/web_server_base.h"

namespace esphome {
namespace settings {

// ESPHome 2026.10 stores web server credentials as const char* instead of std::string, so
// the auth setters take const char*. The accessor keeps the strings alive for the whole
// device lifetime, because the pointers would dangle after the on_load lambda returns.
class WebServerComponentAccessor {
 public:
  explicit WebServerComponentAccessor(web_server_base::WebServerBase *ref) : ref_(ref) {}

// The settings component runs on ESP32 only, and ESP32 keeps the credentials as mutable
// fields (set_auth_username/set_auth_password) in every auth mode.
#ifdef USE_WEBSERVER_AUTH
  void set_auth_username(const std::string &value) {
    username_ = value;
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2026, 10, 0)
    // WebServerBase treats only nullptr as "not set", an empty string keeps auth enabled.
    this->ref_->set_auth_username(username_.empty() ? nullptr : username_.c_str());
#else
    this->ref_->set_auth_username(username_);
#endif
  }
  void set_auth_password(const std::string &value) {
    password_ = value;
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2026, 10, 0)
    this->ref_->set_auth_password(password_.c_str());
#else
    this->ref_->set_auth_password(password_);
#endif
  }
#endif

 private:
  web_server_base::WebServerBase *ref_;
  inline static std::string username_;
  inline static std::string password_;
};

inline WebServerComponentAccessor web_server_access(web_server_base::WebServerBase *ref) {
  return WebServerComponentAccessor(ref);
}

}  // namespace settings
}  // namespace esphome
