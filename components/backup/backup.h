#pragma once

#include "esphome/core/component.h"
#include "esphome/core/controller.h"
#include "esphome/core/version.h"
#include "esphome/core/defines.h"
#include "esphome/components/web_server_base/web_server_base.h"

#include <vector>

extern const uint8_t ESPHOME_BACKUP_DATA[] PROGMEM;
extern const size_t ESPHOME_BACKUP_SIZE;

namespace esphome {
namespace backup {

class Backup : public Component, public AsyncWebHandler {
 public:
  explicit Backup(web_server_base::WebServerBase *base) : base_(base) {}

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::WIFI - 1.0f; }

  void set_username(const char *username) { this->username_ = username; }
  void set_password(const char *password) { this->password_ = password; }

#ifdef USE_WEBSERVER_AUTH
  bool using_auth() { return this->username_ != nullptr && this->password_ != nullptr; }
#endif

  bool canHandle(AsyncWebServerRequest *request)
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 7, 0)
      const
#endif
      override;
  void handleRequest(AsyncWebServerRequest *request) override;

 protected:
  web_server_base::WebServerBase *base_;
  const uint8_t *data_{nullptr};
  size_t size_{0};
  const char *username_{nullptr};
  const char *password_{nullptr};
};

}  // namespace backup
}  // namespace esphome
