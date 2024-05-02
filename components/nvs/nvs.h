#pragma once

#include <cinttypes>
#include <string>
#include <type_traits>
#include <vector>

#if defined(__has_include) && __has_include(<span>)
#include <span>
#endif

#include "nvs_flash.h"
#include "nvs_handle.hpp"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/optional.h"

namespace esphome {
namespace nvs_flash {

#define NVS_TRACE ESP_LOGD

template<typename T>  // NOLINTNEXTLINE(readability-identifier-naming)
struct is_nvs_type : std::integral_constant<bool, std::is_integral<T>::value && !std::is_same<T, bool>::value> {};

class NvsFlash {
 public:
  NvsFlash() {}
  NvsFlash(const char *namespace_name, bool read_write = false) { this->open(namespace_name, read_write); }
  NvsFlash(const std::string &namespace_name, bool read_write = false) { this->open(namespace_name, read_write); }
  ~NvsFlash() { this->close(); }

  bool init() { return nvs_flash_init() == ESP_OK; }

  bool open(const char *namespace_name, nvs_open_mode_t open_mode);
  bool open(const std::string &namespace_name, nvs_open_mode_t open_mode) {
    return this->open(namespace_name.c_str(), open_mode);
  }
  bool open(const char *namespace_name, bool read_write = false) {
    return this->open(namespace_name, read_write ? NVS_READWRITE : NVS_READONLY);
  }
  bool open(const std::string &namespace_name, bool read_write = false) {
    return this->open(namespace_name.c_str(), read_write);
  }

  void close() {
    if (this->handle_) {
      this->handle_.reset();
    }
  }

  bool commit() const {
    if (this->handle_) {
      auto err = this->handle_->commit();
      if (err == ESP_OK) {
        return true;
      }
      NVS_TRACE("nvs", "commit failed: %s", esp_err_to_name(err));
    }
    return false;
  }

  bool erase(const char *key) const {
    if (this->handle_) {
      auto err = this->handle_->erase_item(key);
      if (err == ESP_OK) {
        return true;
      }
      NVS_TRACE("nvs", "erase %s failed: %s", key, esp_err_to_name(err));
    }
    return false;
  }

  bool erase(const std::string &key) const { return this->erase(key.c_str()); }

  template<typename T, enable_if_t<std::is_same<T, std::string>::value, int> = 0>
  optional<T> get(const char *key) const {
    if (!this->handle_) {
      return {};
    }
    return {this->get_string_(key)};
  }

  template<typename T, enable_if_t<std::is_same<T, float>::value, int> = 0> optional<T> get(const char *key) const {
    static_assert(sizeof(float) == sizeof(uint32_t));
    if (!this->handle_) {
      return {};
    }
    union {
      uint32_t u;
      float f;
    } res{};
    auto err = this->handle_->get_item(key, res.u);
    if (err != ESP_OK) {
      NVS_TRACE("nvs", "get float %s failed: %s", key, esp_err_to_name(err));
      return {};
    }
    NVS_TRACE("nvs", "get float %s success: %f", key, res.f);
    return {res.f};
  }

  template<typename T, enable_if_t<std::is_same<T, double>::value, int> = 0> optional<T> get(const char *key) const {
    static_assert(sizeof(double) == sizeof(uint64_t));
    union {
      uint64_t u;
      double d;
    } res{};
    auto err = this->handle_->get_item(key, res.u);
    if (err != ESP_OK) {
      NVS_TRACE("nvs", "get double %s failed: %s", key, esp_err_to_name(err));
      return {};
    }
    NVS_TRACE("nvs", "get double %s success: %f", key, res.d);
    return {res.d};
  }

  template<typename T, enable_if_t<std::is_same<T, bool>::value, int> = 0> optional<T> get(const char *key) const {
    if (!this->handle_) {
      return {};
    }
    uint8_t res{};
    auto err = this->handle_->get_item(key, res);
    if (err != ESP_OK) {
      NVS_TRACE("nvs", "get bool %s failed: %s", key, esp_err_to_name(err));
      return {};
    }
    NVS_TRACE("nvs", "get bool %s success: %s", key, TRUEFALSE(res != 0));
    return {res != 0};
  }

  template<typename T, enable_if_t<is_nvs_type<T>::value, int> = 0> optional<T> get(const char *key) const {
    static_assert(!std::is_same<T, bool>::value);
    if (!this->handle_) {
      return {};
    }
    T res{};
    auto err = this->handle_->get_item(key, res);
    if (err != ESP_OK) {
      NVS_TRACE("nvs", "get nvs_type %s failed: %s", key, esp_err_to_name(err));
      return {};
    }
    NVS_TRACE("nvs", "get nvs_type %s success: %s", key, to_string(res).c_str());
    return {res};
  }

  template<typename T, enable_if_t<is_nvs_type<T>::value, int> = 0> bool set(const char *key, T value) const {
    static_assert(!std::is_same<T, bool>::value);
    if (!this->handle_) {
      return false;
    }
    auto err = this->handle_->set_item(key, value);
    if (err != ESP_OK) {
      NVS_TRACE("nvs", "set nvs_type %s failed: %s", key, esp_err_to_name(err));
      return false;
    }
    NVS_TRACE("nvs", "set nvs_type %s success: %s", key, to_string(value).c_str());
    return true;
  }

  bool set(const char *key, bool value) const {
    if (!this->handle_) {
      return false;
    }
    const uint8_t cv = value ? 1 : 0;
    auto err = this->handle_->set_item(key, cv);
    if (err != ESP_OK) {
      NVS_TRACE("nvs", "set bool %s failed: %s", key, esp_err_to_name(err));
      return false;
    }
    NVS_TRACE("nvs", "set bool %s success: %s", key, TRUEFALSE(value));
    return true;
  }

  bool set(const char *key, float value) const {
    static_assert(sizeof(float) == sizeof(uint32_t));
    if (!this->handle_) {
      return false;
    }
    const auto *cv = static_cast<const uint32_t *>(static_cast<const void *>(&value));
    auto err = this->handle_->set_item(key, *cv);
    if (err != ESP_OK) {
      NVS_TRACE("nvs", "set float %s failed: %s", key, esp_err_to_name(err));
      return false;
    }
    NVS_TRACE("nvs", "set float %s success: %f", key, value);
    return true;
  }

  bool set(const char *key, double value) const {
    static_assert(sizeof(double) == sizeof(uint64_t));
    if (!this->handle_) {
      return false;
    }
    const auto *cv = static_cast<const uint64_t *>(static_cast<const void *>(&value));
    auto err = this->handle_->set_item(key, *cv);
    if (err != ESP_OK) {
      NVS_TRACE("nvs", "set double %s failed: %s", key, esp_err_to_name(err));
      return false;
    }
    NVS_TRACE("nvs", "set double %s success: %f", key, value);
    return true;
  }

  bool set(const char *key, const char *value) const {
    if (!this->handle_) {
      return false;
    }
    auto err = this->handle_->set_string(key, value);
    if (err != ESP_OK) {
      NVS_TRACE("nvs", "set string %s failed: %s", key, esp_err_to_name(err));
      return false;
    }
    NVS_TRACE("nvs", "set string %s success: %s", key, value);
    return true;
  }

  bool set(const char *key, const std::string &value) const { return this->set(key, value.c_str()); }

  bool set(const char *key, const void *value, size_t size) const {
    if (!this->handle_) {
      return false;
    }
    auto err = this->handle_->set_blob(key, value, size);
    if (err != ESP_OK) {
      NVS_TRACE("nvs", "set blob %s failed: %s", key, esp_err_to_name(err));
      return false;
    }
    NVS_TRACE("nvs", "set blob %s success: %s", key,
              format_hex_pretty(static_cast<const uint8_t *>(value), size).c_str());
    return true;
  }

  bool set(const char *key, const std::vector<uint8_t> &value) const {
    return this->set(key, value.data(), value.size());
  }

#if __cpp_lib_span >= 202002L
  template<class T, std::size_t N> bool set(const char *key, std::span<T, N> value) const {
    return this->set(key, value.data(), value.size_bytes());
  }
#endif

  bool reset() const { return this->handle_->erase_all() == ESP_OK; }

 protected:
  std::unique_ptr<nvs::NVSHandle> handle_;
  optional<std::string> get_string_(const char *key) const;
  optional<std::vector<uint8_t>> get_blob_(const char *key) const;
};

}  // namespace nvs_flash
}  // namespace esphome
