#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#include "esp_partition.h"
#include "esp_core_dump.h"
#include "esp_flash.h"

#include "coredump.h"

namespace esphome {
namespace coredump {

static const char *const TAG = "coredump";

constexpr static size_t PARTITION_SIZE = 0x10000;
constexpr static size_t CHUNK_SIZE = 0x400;

void Coredump::dump_config() {
  ESP_LOGCONFIG(TAG, "Coredump:");
  ESP_LOGCONFIG(TAG, "  Index URL: %s", this->index_url_);
  ESP_LOGCONFIG(TAG, "  Erase URL: %s", this->erase_url_);
  ESP_LOGCONFIG(TAG, "  Download URL: %s", this->download_url_);
}

void Coredump::setup() {
  ESP_LOGCONFIG(TAG, "Setting up coredump handler...");
  this->base_->init();
  this->base_->add_handler(this);
}

bool Coredump::canHandle(AsyncWebServerRequest *request) {
  if (request->method() != HTTP_GET) {
    return false;
  }
  auto url = request->url();
  if (url == this->index_url_) {
    return true;
  }
  if (url == this->download_url_) {
    return true;
  }
  if (url == this->erase_url_) {
    return true;
  }
  if (url == this->crash_url_) {
    return true;
  }
  return false;
}

void Coredump::handleRequest(AsyncWebServerRequest *request) {
  auto url = request->url();
  if (url == this->index_url_) {
    this->index_(request);
  } else if (url == this->download_url_) {
    this->download_(request);
  } else if (url == this->erase_url_) {
    this->erase_(request);
  } else if (url == this->crash_url_) {
    this->crash_(request);
  }
}

namespace {

inline void write_html_chunk(AsyncWebServerRequest *request, const std::string &html) {
  httpd_resp_send_chunk(*request, html.c_str(), html.length());
}

inline void write_html_begin(AsyncWebServerRequest *request, const char *css_url, const char *redirect_url = nullptr) {
  auto tags = str_sprintf(R"(<link rel="stylesheet" href="%s">)", css_url);
  if (redirect_url) {
    tags.append(str_sprintf(R"(<meta http-equiv="refresh" content="3;url=%s">)", redirect_url));
  }
  write_html_chunk(request, str_sprintf(R"(<!DOCTYPE html>
    <html><head>
    <meta charset="utf-8">
    <meta name=viewport content="width=device-width,initial-scale=1,user-scalable=no">
    %s
    <title>%s</title>
    </head><body><main class="container">)",
                                        tags.c_str(), TAG));
}

inline void write_html_end(AsyncWebServerRequest *request) { write_html_chunk(request, R"(</main></body></html>)"); }

inline void write_html_link(AsyncWebServerRequest *request, const char *name, const char *url) {
  write_html_chunk(request, str_sprintf(R"(<a href="%s" role="button">%s</a> )", url, name));
}

inline void write_html_message(AsyncWebServerRequest *request, const char *message) {
  write_html_chunk(request, str_sprintf(R"(<article>%s</article>)", message));
}

}  // namespace

void Coredump::crash_(AsyncWebServerRequest *request) {
  request->beginResponse(200, "text/html");

  write_html_begin(request, this->css_url_, this->index_url_);
  write_html_message(request, "Generated");
  write_html_link(request, "Return", this->index_url_);
  write_html_end(request);

  httpd_resp_send_chunk(*request, nullptr, 0);

  ESP_LOGI(TAG, "Now generating crash and reboot...");
  esphome::delay(1000);
  assert(0);  // NOLINT cert-dcl03-c
  // while (true) {(void) 0;}
}

void Coredump::index_(AsyncWebServerRequest *request) {
  request->beginResponse(200, "text/html");
  write_html_begin(request, this->css_url_);

  write_html_message(request, "Select action");

  esp_core_dump_summary_t summary;
  if (esp_core_dump_get_summary(&summary) == ESP_OK) {
    write_html_link(request, "Download", this->download_url_);
    write_html_link(request, "Erase", this->erase_url_);
  } else {
    write_html_link(request, "Test Crash", this->crash_url_);
  }

  write_html_end(request);
  // request->send(response);
  httpd_resp_send_chunk(*request, nullptr, 0);
}

void Coredump::erase_(AsyncWebServerRequest *request) {
  request->beginResponse(200, "text/html");

  write_html_begin(request, this->css_url_, this->index_url_);

  esp_core_dump_image_erase();
  if (esp_core_dump_image_check() == ESP_OK) {
    write_html_message(request, "Failed");
  } else {
    write_html_message(request, "Success");
  }
  write_html_link(request, "Return", this->index_url_);
  write_html_end(request);

  httpd_resp_send_chunk(*request, nullptr, 0);
}

void Coredump::download_(AsyncWebServerRequest *request) {
  if (esp_core_dump_image_check() != ESP_OK) {
    return request->send(404);
  }

  size_t addr;
  size_t size;
  if (esp_core_dump_image_get(&addr, &size) != ESP_OK) {
    return request->send(503, "text/plain", "Failed to get core dump");
  }

  auto *response = request->beginResponse(200, "application/octet-stream");
  response->addHeader("Content-Disposition", "attachment;filename=coredump.elf");

  char buf[CHUNK_SIZE];
  for (size_t i = 0; i < size; i += sizeof(buf)) {
    const size_t to_send = std::min(sizeof(buf), size - i);
    if (esp_flash_read(nullptr, buf, addr + i, to_send) != ESP_OK) {
      ESP_LOGW(TAG, "esp_flash_read failed. Core dump truncated");
      // TODO do we need to write something to output file?
      break;
    }
    httpd_resp_send_chunk(*request, buf + (i == 0 ? 20 : 0), to_send - (i == 0 ? 20 : 0));
  }

  // request->send(response);
  // Send empty chunk to signal HTTP response completion
  httpd_resp_send_chunk(*request, nullptr, 0);
}

void Coredump::css_(AsyncWebServerRequest *request) {
  auto *response = request->beginResponse(200, "text/css");
  // write_html_chunk(request, s);
  request->send(response);
}

}  // namespace coredump
}  // namespace esphome
