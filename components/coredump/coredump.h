#pragma once

#include "esphome/core/component.h"
#include "esphome/components/web_server_base/web_server_base.h"

#include <vector>

namespace esphome {
namespace coredump {

class Coredump : public Component, public AsyncWebHandler {
 public:
  explicit Coredump(web_server_base::WebServerBase *base) : base_(base) {}

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::WIFI - 1.0f; }

  bool canHandle(AsyncWebServerRequest *request) override;
  void handleRequest(AsyncWebServerRequest *request) override;

 protected:
  web_server_base::WebServerBase *base_;
  const char *index_url_{"/coredump/"};
  const char *erase_url_{"/coredump/erase"};
  const char *download_url_{"/coredump/coredump.elf"};
  const char *crash_url_{"/coredump/crash"};
  const char *css_url_{"https://cdn.jsdelivr.net/npm/@picocss/pico@1/css/pico.min.css"};

  void css_(AsyncWebServerRequest *request);
  void index_(AsyncWebServerRequest *request);
  void erase_(AsyncWebServerRequest *request);
  void crash_(AsyncWebServerRequest *request);
  void download_(AsyncWebServerRequest *request);
};

}  // namespace coredump
}  // namespace esphome
