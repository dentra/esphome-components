
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/util.h"

#include <cstdlib>

#include "backup.h"

namespace esphome {
namespace backup {

static const char *const TAG = "backup";
static const char *const URL = "/config.yaml";

void Backup::setup() {
  ESP_LOGCONFIG(TAG, "Setting up backup handler...");
  this->base_->init();
  this->base_->add_handler(this);
}

void Backup::dump_config() {
  ESP_LOGCONFIG(TAG, "Backup:");
  ESP_LOGCONFIG(TAG, "  URL path is %s", URL);
#ifdef USE_WEBSERVER_AUTH
  if (this->using_auth()) {
    ESP_LOGCONFIG(TAG, "  Basic authentication enabled");
  }
#endif
}

bool Backup::canHandle(AsyncWebServerRequest *request)
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 7, 0)
    const
#endif
{
  return request->method() == HTTP_GET && request->url() == URL;
}

void Backup::handleRequest(AsyncWebServerRequest *request) {
#ifdef USE_WEBSERVER_AUTH
  if (this->using_auth() && !request->authenticate(this->username_, this->password_)) {
    return request->requestAuthentication();
  }
#endif
#if defined(USE_ESP_IDF) && ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 7, 0)
  auto *response = request->beginResponse(200, "plain/text;charset=UTF-8", ESPHOME_BACKUP_DATA, ESPHOME_BACKUP_SIZE);
#else
  auto *response = request->beginResponse_P(200, "plain/text;charset=UTF-8", ESPHOME_BACKUP_DATA, ESPHOME_BACKUP_SIZE);
#endif
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

}  // namespace backup
}  // namespace esphome
