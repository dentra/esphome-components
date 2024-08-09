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

#define NVS_TRACE ESP_LOGV

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
      // The handle is automatically closed on destruction.
      this->handle_.reset();
    }
  }

  bool commit() const;

  bool erase(const char *key) const;
  bool erase(const std::string &key) const { return this->erase(key.c_str()); }

  template<typename T, enable_if_t<std::is_same<T, std::string>::value, int> = 0>
  optional<T> get(const char *key) const {
    return this->get_string_(key);
  }
  template<typename T, enable_if_t<std::is_same<T, std::string>::value, int> = 0>
  optional<T> get(const std::string &key) const {
    return this->get_string_(key.c_str());
  }

  template<typename T, enable_if_t<std::is_same<T, float>::value, int> = 0> optional<T> get(const char *key) const {
    static_assert(sizeof(float) == sizeof(uint32_t));
    return this->get_float_(key);
  }
  template<typename T, enable_if_t<std::is_same<T, float>::value, int> = 0>
  optional<T> get(const std::string &key) const {
    static_assert(sizeof(float) == sizeof(uint32_t));
    return this->get_float_(key.c_str());
  }

  template<typename T, enable_if_t<std::is_same<T, double>::value, int> = 0> optional<T> get(const char *key) const {
    static_assert(sizeof(double) == sizeof(uint64_t));
    return this->get_double_(key);
  }
  template<typename T, enable_if_t<std::is_same<T, double>::value, int> = 0>
  optional<T> get(const std::string &key) const {
    static_assert(sizeof(double) == sizeof(uint64_t));
    return this->get_double_(key.c_str());
  }

  template<typename T, enable_if_t<std::is_same<T, bool>::value, int> = 0> optional<T> get(const char *key) const {
    return this->get_bool_(key);
  }
  template<typename T, enable_if_t<std::is_same<T, bool>::value, int> = 0>
  optional<T> get(const std::string &key) const {
    return this->get_bool_(key.c_str());
  }

  template<typename T, enable_if_t<is_nvs_type<T>::value, int> = 0> optional<T> get(const char *key) const {
    static_assert(!std::is_same<T, bool>::value);  // check that is not bool
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
    static_assert(!std::is_same<T, bool>::value);  // check that is not bool
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

  bool set(const char *key, bool value) const { return this->set_bool_(key, value); }
  bool set(const std::string &key, bool value) const { return this->set(key.c_str(), value); }

  bool set(const char *key, float value) const { return this->set_float_(key, value); }
  bool set(const std::string &key, float value) const { return this->set(key.c_str(), value); }

  bool set(const char *key, double value) const { return this->set_double_(key, value); }
  bool set(const std::string &key, double value) const { return this->set(key.c_str(), value); }

  bool set(const char *key, const char *value) const { return this->set_string_(key, value); }
  bool set(const std::string &key, const char *value) const { return this->set(key.c_str(), value); }
  bool set(const char *key, const std::string &value) const { return this->set(key, value.c_str()); }
  bool set(const std::string &key, const std::string &value) const { return this->set(key.c_str(), value.c_str()); }

  bool set(const char *key, const void *value, size_t size) const { return this->set_blob_(key, value, size); }
  bool set(const std::string &key, const void *value, size_t size) const { return this->set(key.c_str(), value, size); }
  bool set(const char *key, const std::vector<uint8_t> &value) const {
    return this->set(key, value.data(), value.size());
  }
  bool set(const std::string &key, const std::vector<uint8_t> &value) const { return this->set(key.c_str(), value); }

#if __cpp_lib_span >= 202002L
  template<class T, std::size_t N> bool set(const char *key, std::span<T, N> value) const {
    return this->set(key, value.data(), value.size_bytes());
  }
  template<class T, std::size_t N> bool set(const std::string &key, std::span<T, N> value) const {
    return this->set(key.c_str(), value.data(), value.size_bytes());
  }
#endif

  bool reset() const { return this->handle_->erase_all() == ESP_OK; }

  /// Get the blob from nvs.
  /// if data is nullptr then returned blob size.
  size_t get_blob(const char *key, uint8_t *data, size_t size) const;
  size_t get_blob(const std::string &key, uint8_t *data, size_t size) const {
    return this->get_blob(key.c_str(), data, size);
  }
  optional<std::vector<uint8_t>> get_blob(const char *key) const;
  optional<std::vector<uint8_t>> get_blob(const std::string &key) const { return this->get_blob(key.c_str()); }

 protected:
  std::unique_ptr<nvs::NVSHandle> handle_;
  bool set_blob_(const char *key, const void *value, size_t size) const;

  optional<std::string> get_string_(const char *key) const;
  bool set_string_(const char *key, const char *value) const;

  optional<float> get_float_(const char *key) const;
  bool set_float_(const char *key, float value) const;

  optional<double> get_double_(const char *key) const;
  bool set_double_(const char *key, double value) const;

  optional<bool> get_bool_(const char *key) const;
  bool set_bool_(const char *key, bool value) const;
};

}  // namespace nvs_flash
}  // namespace esphome
