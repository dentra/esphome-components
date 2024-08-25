#include <cinttypes>

#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/application.h"
#include "esphome/core/defines.h"
#include "esphome/core/version.h"

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
  const auto title = (App.get_friendly_name().empty() ? App.get_name() : App.get_friendly_name()) + " coredump";

  std::string html_begin = R"(<!DOCTYPE html>
    <html><head>
    <meta charset="utf-8">
    <meta name=viewport content="width=device-width,initial-scale=1,user-scalable=no">
    <meta name="color-scheme" content="light dark">)";
  html_begin += tags;
  html_begin += "<title>" + title + "</title>";
  html_begin += "</head><body><main><h1>" + title + "</h1>";
#ifdef ESPHOME_PROJECT_NAME
  html_begin += "<p>" ESPHOME_PROJECT_NAME ": " ESPHOME_PROJECT_VERSION "</p>";
#endif
  html_begin += "<p>ESPHome: " ESPHOME_VERSION "</p>";
  html_begin += "<p>Compilation Time: " + App.get_compilation_time() + "</p>";

  write_html_chunk(request, html_begin.c_str());
}

inline void write_html_end(AsyncWebServerRequest *request) { write_html_chunk(request, R"(</main></body></html>)"); }

inline void write_html_link(AsyncWebServerRequest *request, const char *name, const char *url) {
  write_html_chunk(request, str_sprintf(R"(<a href="%s" role="button">%s</a> )", url, name));
}

inline void write_html_message(AsyncWebServerRequest *request, const std::string &message) {
  write_html_chunk(request, "<article>" + message + "</article>");
}

inline void write_html_message(AsyncWebServerRequest *request, const std::string &message1,
                               const std::string &message2) {
  write_html_chunk(request, "<article>" + message1 + ": " + message2 + "</article>");
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

  esp_core_dump_summary_t summary;
  if (esp_core_dump_get_summary(&summary) == ESP_OK) {
#if ESP_IDF_VERSION_MAJOR > 5
    char panic_reason[200];
    if (esp_core_dump_get_panic_reason(panic_reason, sizeof(panic_reason))) {
      write_html_message(request, "Panic reason", panic_reason);
    }
#endif
    write_html_message(request, "Exc TCB", str_snprintf("%08" PRIx32, 8, summary.exc_tcb));
    write_html_message(request, "Exc Task", summary.exc_task);
    write_html_message(request, "Exc PC", str_snprintf("%08" PRIx32, 8, summary.exc_pc));

    if (summary.exc_bt_info.corrupted) {
      write_html_message(request, "Backtrace is corrupted");
    } else {
      std::string backtrace;
      for (int i = 0; i < summary.exc_bt_info.depth; i++) {
        backtrace += str_snprintf("%08" PRIx32 " ", 9, summary.exc_bt_info.bt[i]);
      }
      write_html_message(request, "Backtrace", backtrace);
    }

    write_html_message(request, "Core dump version", str_snprintf("0x%" PRIx32, 10, summary.core_dump_version));
    write_html_message(request, "App ELF SHA2", reinterpret_cast<const char *>(summary.app_elf_sha256));

    write_html_message(request, "Select action");
    write_html_link(request, "Download", this->download_url_);
    write_html_link(request, "Erase", this->erase_url_);
  } else {
    write_html_message(request, "Great! There is no crash report yet.");
#ifdef USE_COREDUMP_ENABLE_TEST_CRASH
    write_html_link(request, "Click on this button to cause a crash", this->crash_url_);
#endif
  }

  write_html_link(request, "Go to Home", "/");

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
#ifdef ESPHOME_PROJECT_NAME
  response->addHeader("Content-Disposition", "attachment;filename=coredump-" ESPHOME_PROJECT_VERSION ".elf");
#else
  response->addHeader("Content-Disposition",
                      ("attachment;filename=coredump-" + App.get_compilation_time() + ".elf").c_str());
#endif

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
