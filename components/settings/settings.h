#pragma once

#include <cstdlib>
#include "esphome/core/application.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include "../nvs/nvs.h"

extern const uint8_t ESPHOME_WEBSERVER_SETTINGS_HTML[] PROGMEM;
extern const size_t ESPHOME_WEBSERVER_SETTINGS_HTML_SIZE;

extern const uint8_t ESPHOME_WEBSERVER_SETTINGS_JS[] PROGMEM;
extern const size_t ESPHOME_WEBSERVER_SETTINGS_JS_SIZE;

namespace esphome {

namespace settings {

#define SETTINGS_TRACE ESP_LOGD

enum VariableType : uint8_t {
  VAR_TYPE_UNKNOWN = 0,
  VAR_TYPE_INT8 = 1,
  VAR_TYPE_UINT8 = 2,
  VAR_TYPE_INT16 = 3,
  VAR_TYPE_UINT16 = 4,
  VAR_TYPE_INT32 = 5,
  VAR_TYPE_UINT32 = 6,
  VAR_TYPE_INT64 = 7,
  VAR_TYPE_UINT64 = 8,
  VAR_TYPE_STR = 9,
  VAR_TYPE_BOOL = 10,
  VAR_TYPE_FLOAT = 11,
  VAR_TYPE_DOUBLE = 12,
  VAR_TYPE_PASSWORD = 13,
  VAR_TYPE_MAC = 14,
  VAR_TYPE_PIN = 15,
  VAR_TYPE_TIMEOUT = 16,
};

struct VarInfo {
  const char *key;
  VariableType type;
  const char *name;
  const char *section;
  const char *help;
  float min;
  float max;
  uint8_t accuracy_decimals;
  union {
    int8_t (*i8)();
    uint8_t (*u8)();
    int16_t (*i16)();
    uint16_t (*u16)();
    int32_t (*i32)();
    uint32_t (*u32)();
    int64_t (*i64)();
    uint64_t (*u64)();
    std::string (*s)();
    bool (*b)();
    float (*f)();
    double (*d)();
  } getter;
};

class Settings : public AsyncWebHandler, public Component {
 public:
  Settings(web_server_base::WebServerBase *base) : base_(base) {}

  void dump_config() override;
  void setup() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  bool canHandle(AsyncWebServerRequest *request) override;
  void handleRequest(AsyncWebServerRequest *request) override;

  void register_variable(VarInfo &&info) { this->items_.push_back(std::move(info)); }

  void load(void (*on_load)(const nvs_flash::NvsFlash &nvs));

  void set_base_url(const char *value) { this->base_url_ = value; }
  const char *get_base_url() const { return this->base_url_.c_str(); }
  void set_username(const char *username) { this->username_ = username; }
  void set_password(const char *password) { this->password_ = password; }
  void set_menu_url(const char *value) { this->menu_url_ = value; }

  /// find settings by key (not hash) and update it value.
  bool set_value(const std::string &key, const std::string &value);

 protected:
  nvs_flash::NvsFlash nvs_;
  const char *menu_url_{};
  const char *username_{};
  const char *password_{};
  std::string base_url_{"/settings"};
  web_server_base::WebServerBase *base_{};
  std::vector<VarInfo> items_;
  void handle_html_(AsyncWebServerRequest *request);
  void handle_js_(AsyncWebServerRequest *request);
  void handle_json_(AsyncWebServerRequest *request);
  void handle_save_(AsyncWebServerRequest *request);
  void handle_reset_(AsyncWebServerRequest *request);
  void redirect_home_(AsyncWebServerRequest *request);

  std::string url_(const char *path);

  void reboot_();

  std::string get_json_value_(const VarInfo &v) const;
  bool save_pv_(const VarInfo &x, const char *param) const;

  template<typename T> inline T get_value_(const char *key, T (*const fn)()) const {
    SETTINGS_TRACE(this->get_component_source(), "get_value_ for %s", key);
    auto opt = this->nvs_.get<T>(key);
    if (opt.has_value()) {
      return opt.value();
    }
    SETTINGS_TRACE(this->get_component_source(), "no stored value found");
    return fn();
  }

  template<typename T> inline void set_value_(const char *key, T (*const fn)(), T val) const {
    if (fn() == val) {
      this->nvs_.erase(key);
    } else {
      this->nvs_.set(key, val);
    }
  }

  template<typename T>
  inline bool parse_number_set_value_(const char *key, const char *param, VariableType type, T (*const fn)()) const {
    const auto opt = parse_number<T>(param);
    if (!opt.has_value()) {
      ESP_LOGW(this->get_component_source(), "Invalid %s number of type %u value: %s", key, type, param);
      return false;
    }
    this->set_value_(key, fn, opt.value());
    return true;
  }
};

}  // namespace settings
}  // namespace esphome
