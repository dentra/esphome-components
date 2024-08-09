#include "esphome/core/defines.h"
#ifdef USE_ESP_IDF
#include "esphome/core/log.h"
#include "esp_http_client.h"
#include "net.h"

namespace esphome {
namespace dtu {

static const char *const TAG = "dtu.net";

bool check_url_available(const char *url) {
  esp_http_client_config_t config{};
  config.url = url;
  config.buffer_size_tx = 2048;
  esp_http_client_handle_t client = esp_http_client_init(&config);
  esp_err_t err = ESP_OK;
  do {
    err = esp_http_client_perform(client);
  } while (err == ESP_ERR_HTTP_EAGAIN);
  ESP_LOGD(TAG, "Perform result for %s: %s", url, esp_err_to_name(err));
  if (err == ESP_OK) {
    const auto status_code = esp_http_client_get_status_code(client);
    ESP_LOGD(TAG, "Status code for %s: %d", url, status_code);
    err = status_code == HttpStatus_Ok ? ESP_OK : ESP_FAIL;
  }
  esp_http_client_cleanup(client);
  ESP_LOGD(TAG, "Checking %s available: %s", url, YESNO(err == ESP_OK));
  return err == ESP_OK;
}

}  // namespace dtu
}  // namespace esphome
#endif
