#include "esphome/core/log.h"
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
// #include "esphome/"

#include "json_writer.h"
#include "settings.h"

namespace esphome {
namespace settings {

static const char *const TAG = "settings";
static const char *const NVS_NS = TAG;

void Settings::dump_config() {
  ESP_LOGCONFIG(TAG, "Settings:");
  ESP_LOGCONFIG(TAG, "  Base URL: %s", this->base_url_.c_str());
}

void Settings::setup() {
  ESP_LOGCONFIG(TAG, "Setting up settings handler...");
  this->base_->init();
  this->base_->add_handler(this);
}

std::string Settings::url_(const char *path) {
  std::string res = this->base_url_;
  if (*res.rbegin() != '/') {
    res += '/';
  }
  res += path;
  return res;
}

void Settings::redirect_home_(AsyncWebServerRequest *request) {
  if (*this->base_url_.rbegin() != '/') {
    request->redirect(this->base_url_ + '/');
  } else {
    request->redirect(this->base_url_);
  }
}

bool Settings::canHandle(AsyncWebServerRequest *request) {
#ifdef USE_ARDUINO
  // arduino returns String but not std::string
  return str_startswith(request->url().c_str(), this->base_url_);
#else
  return str_startswith(request->url(), this->base_url_);
#endif
}

void Settings::handleRequest(AsyncWebServerRequest *request) {  // NOLINT(readability-non-const-parameter)
  SETTINGS_TRACE(TAG, "Handle request method %u, url: %s", request->method(), request->url().c_str());

  if (request->method() == HTTP_POST) {
    if (request->url() == this->url_("reset")) {
      this->handle_reset_(request);
    } else {
      this->handle_save_(request);
    }
    return;
  }

  if (request->method() != HTTP_GET) {
    return;
  }

  if (request->url() == this->url_("settings.json")) {
    this->handle_load_(request);
    return;
  }

  if (*this->base_url_.rbegin() != '/') {
    this->redirect_home_(request);
    return;
  }

  this->handle_base_(request);
}

void Settings::handle_base_(AsyncWebServerRequest *request) {  // NOLINT(readability-non-const-parameter)
  auto *response =
      request->beginResponse_P(200, "text/html", ESPHOME_WEBSERVER_SETTINGS_HTML, ESPHOME_WEBSERVER_SETTINGS_HTML_SIZE);
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

std::string Settings::get_json_value_(const VarInfo &v) const {
  switch (v.type) {
    case VAR_TYPE_PIN:
    case VAR_TYPE_INT8:
      return to_string(this->get_value_<int8_t>(v.key, v.getter.i8));

    case VAR_TYPE_UINT8:
      return to_string(this->get_value_<uint8_t>(v.key, v.getter.u8));

    case VAR_TYPE_INT16:
      return to_string(this->get_value_<int16_t>(v.key, v.getter.i16));

    case VAR_TYPE_UINT16:
      return to_string(this->get_value_<uint16_t>(v.key, v.getter.u16));

    case VAR_TYPE_INT32:
      return to_string(this->get_value_<int32_t>(v.key, v.getter.i32));

    case VAR_TYPE_UINT32:
      return to_string(this->get_value_<uint32_t>(v.key, v.getter.u32));

    case VAR_TYPE_INT64:
      return to_string(this->get_value_<int64_t>(v.key, v.getter.i64));

    case VAR_TYPE_UINT64:
      return to_string(this->get_value_<uint64_t>(v.key, v.getter.u64));

    case VAR_TYPE_FLOAT:
      return to_string(this->get_value_<float>(v.key, v.getter.f));

    case VAR_TYPE_DOUBLE:
      return to_string(this->get_value_<double>(v.key, v.getter.d));

    case VAR_TYPE_TIMEOUT:
      return to_string(this->get_value_<uint32_t>(v.key, v.getter.u32) / 1000);

    case VAR_TYPE_MAC:
      return mac_to_json_string(this->get_value_<uint64_t>(v.key, v.getter.u64));

    case VAR_TYPE_BOOL:
      return to_string(this->get_value_<bool>(v.key, v.getter.b));

    case VAR_TYPE_STR:
      return "\"" + json_escape(this->get_value_<std::string>(v.key, v.getter.s)) + "\"";

    case VAR_TYPE_PASSWORD:
      return "\"\"";

    default:
      ESP_LOGW(TAG, "Unsupported %s type %u", v.key, v.type);
      return "null";
  }
}

void Settings::handle_load_(AsyncWebServerRequest *request) {  // NOLINT(readability-non-const-parameter)
  SETTINGS_TRACE(TAG, "Handle json...");
  this->nvs_.open(NVS_NS);
  auto s = JsonWriter(request->beginResponseStream("application/json"));
  s.start_object();
  s.add_kv("t", App.get_name());
  // #ifdef USE_ESP_IDF
  //   s.add_kv("m", "get");
  // #endif
  s.start_array("v");
  bool is_first = true;  // NOLINT(misc-const-correctness)
  for (auto const &x : this->items_) {
    s.add_comma(!is_first);
    s.start_object();

    s.add_kv("key", x.key);
    s.add_kv("type", x.type);
    s.add_kv("section", x.section);
    s.add_kv("name", x.name);
    s.add_kv("help", x.help);
    if (!std::isnan(x.min)) {
      if (x.type == VAR_TYPE_FLOAT || x.type == VAR_TYPE_DOUBLE) {
        s.add_kv("min", x.min);
      } else {
        s.add_kv("min", static_cast<int>(x.min));
      }
    }
    if (!std::isnan(x.max)) {
      if (x.type == VAR_TYPE_FLOAT || x.type == VAR_TYPE_DOUBLE) {
        s.add_kv("max", x.max);
      } else {
        s.add_kv("max", static_cast<int>(x.max));
      }
    }
    if (x.accuracy_decimals > 0) {
      s.add_kv("accuracy_decimals", x.accuracy_decimals);
    }
    auto value = this->get_json_value_(x);
    s.add_kv_nq("value", value, false);
    s.end_object();
    is_first = false;
    if (x.type == VariableType::VAR_TYPE_PASSWORD) {
      SETTINGS_TRACE(TAG, "Processed %s: %s", x.key, "**********");
    } else {
      SETTINGS_TRACE(TAG, "Processed %s: %s", x.key, value.c_str());
    }
  }
  s.end_array();
  s.end_object();
  request->send(s);
  this->nvs_.close();
}

bool Settings::save_pv_(const VarInfo &x, const char *param) const {
  switch (x.type) {
    case VAR_TYPE_PIN:
    case VAR_TYPE_INT8:
      return this->parse_number_set_value_<int8_t>(x.key, param, x.type, x.getter.i8);
    case VAR_TYPE_UINT8:
      return this->parse_number_set_value_<uint8_t>(x.key, param, x.type, x.getter.u8);
    case VAR_TYPE_INT16:
      return this->parse_number_set_value_<int16_t>(x.key, param, x.type, x.getter.i16);
    case VAR_TYPE_UINT16:
      return this->parse_number_set_value_<uint16_t>(x.key, param, x.type, x.getter.u16);
    case VAR_TYPE_INT32:
      return this->parse_number_set_value_<int32_t>(x.key, param, x.type, x.getter.i32);
    case VAR_TYPE_UINT32:
      return this->parse_number_set_value_<uint32_t>(x.key, param, x.type, x.getter.u32);
    case VAR_TYPE_INT64:
      return this->parse_number_set_value_<int64_t>(x.key, param, x.type, x.getter.i64);
    case VAR_TYPE_UINT64:
      return this->parse_number_set_value_<uint64_t>(x.key, param, x.type, x.getter.u64);
    case VAR_TYPE_FLOAT:
      return this->parse_number_set_value_<float>(x.key, param, x.type, x.getter.f);
    case VAR_TYPE_DOUBLE:
      return this->parse_number_set_value_<double>(x.key, param, x.type, x.getter.d);

    case VAR_TYPE_TIMEOUT: {
      const auto opt = parse_number<uint32_t>(param);
      if (!opt.has_value()) {
        ESP_LOGW(TAG, "Invalid %s timeout value: %s", x.key, param);
        return false;
      }
      this->set_value_<uint32_t>(x.key, x.getter.u32, opt.value() * 1000);
      return true;
    }

    case VAR_TYPE_MAC: {
      uint64_t val;
      if (!parse_mac(param, &val)) {
        ESP_LOGW(TAG, "Invalid %s mac value: %s", x.key, param);
        return false;
      }
      this->set_value_<uint64_t>(x.key, x.getter.u64, val);
      return true;
    }

    case VAR_TYPE_BOOL: {
      const char ch = tolower(param[0]);
      // t - is first letter of True, f - irst letter of False
      if (ch != 't' && ch != 'f') {
        ESP_LOGW(TAG, "Invalid %s bool value: %s", x.key, param);
        return false;
      }
      this->set_value_<bool>(x.key, x.getter.b, ch == 't');
      return true;
    }

    case VAR_TYPE_STR:
    case VAR_TYPE_PASSWORD: {
      this->set_value_<std::string>(x.key, x.getter.s, param);
      return true;
    }

    default:
      ESP_LOGW(TAG, "Unsupported %s type %u", x.key, x.type);
      return false;
  }
}

void Settings::handle_save_(AsyncWebServerRequest *request) {  // NOLINT(readability-non-const-parameter)
  SETTINGS_TRACE(TAG, "Saving changes...");
  this->nvs_.open(NVS_NS, true);

  size_t changes = 0;
  for (auto const &x : this->items_) {
    SETTINGS_TRACE(TAG, "Processing %s", x.key);
    const auto *web_param = request->getParam(x.key);
    if (web_param) {
      SETTINGS_TRACE(TAG, "Found value %s", web_param->value().c_str());
      if (this->save_pv_(x, web_param->value().c_str())) {
        ESP_LOGD(TAG, "New %s value %s", x.key, web_param->value().c_str());
        changes++;
      }
    }
  }

  if (changes > 0) {
    ESP_LOGD(TAG, "%zu change(s) was made", changes);
    this->nvs_.commit();
    this->reboot_();
  }

  this->nvs_.close();

  this->redirect_home_(request);
}

void Settings::handle_reset_(AsyncWebServerRequest *request) {  // NOLINT(readability-non-const-parameter)
  global_preferences->reset();
  this->reboot_();
  this->redirect_home_(request);
}

void Settings::reboot_() {
  this->set_timeout(1000, []() { App.safe_reboot(); });
}

void Settings::load(void (*on_load)(const nvs_flash::NvsFlash &nvs)) {
  this->nvs_.open(NVS_NS);
  on_load(this->nvs_);
  this->nvs_.close();
}

}  // namespace settings
}  // namespace esphome
