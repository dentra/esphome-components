#include "nvs.h"
#include "esphome/core/log.h"

namespace esphome {
namespace nvs_flash {
static const char *const TAG = "settings";

bool NvsFlash::open(const char *namespace_name, nvs_open_mode_t open_mode) {
  NVS_TRACE(TAG, "Opening nvs namespace %s, mode %u...", namespace_name, open_mode);
  esp_err_t err{};
  this->handle_ = nvs::open_nvs_handle(namespace_name, open_mode, &err);
  if (err == ESP_OK) {
    return true;
  }
  ESP_LOGW(TAG, "open_nvs_handle failed: %s", esp_err_to_name(err));
  return false;
}

bool NvsFlash::commit() const {
  if (this->handle_) {
    auto err = this->handle_->commit();
    if (err == ESP_OK) {
      return true;
    }
    NVS_TRACE(TAG, "commit failed: %s", esp_err_to_name(err));
  }
  return false;
}

bool NvsFlash::erase(const char *key) const {
  if (this->handle_) {
    auto err = this->handle_->erase_item(key);
    if (err == ESP_OK) {
      return true;
    }
    NVS_TRACE(TAG, "erase %s failed: %s", key, esp_err_to_name(err));
  }
  return false;
}

optional<std::string> NvsFlash::get_string_(const char *key) const {
  if (!this->handle_) {
    return {};
  }

  size_t len{};
  esp_err_t err = this->handle_->get_item_size(nvs::ItemType::SZ, key, len);
  if (err != ESP_OK) {
    NVS_TRACE(TAG, "get_item_size %s: %s - the key might not be set yet", key, esp_err_to_name(err));
    return {};
  }

  std::string res;
  res.resize(len - 1);
  err = this->handle_->get_string(key, res.data(), len);
  if (err != ESP_OK) {
    NVS_TRACE(TAG, "get string %s failed: %s", key, esp_err_to_name(err));
    return {};
  }
  NVS_TRACE(TAG, "get string %s success: %s", key, res.c_str());
  return res;
}

bool NvsFlash::set_string_(const char *key, const char *value) const {
  if (!this->handle_) {
    return false;
  }
  auto err = this->handle_->set_string(key, value);
  if (err != ESP_OK) {
    NVS_TRACE(TAG, "set string %s failed: %s", key, esp_err_to_name(err));
    return false;
  }
  NVS_TRACE(TAG, "set string %s success: %s", key, value);
  return true;
}

size_t NvsFlash::get_blob(const char *key, uint8_t *data, size_t size) const {
  if (!this->handle_) {
    return false;
  }

  if (data == nullptr) {
    esp_err_t err = this->handle_->get_item_size(nvs::ItemType::BLOB, key, size);
    if (err != ESP_OK) {
      NVS_TRACE(TAG, "get_item_size %s: %s - the key might not be set yet", key, esp_err_to_name(err));
      return {};
    }
  }

  esp_err_t err = this->handle_->get_blob(key, data, size);
  if (err != ESP_OK) {
    NVS_TRACE(TAG, "get blob %s failed: %s", key, esp_err_to_name(err));
    return false;
  }
  NVS_TRACE(TAG, "get blob %s success: %s", key, format_hex_pretty(data, size).c_str());
  return size;
}

optional<std::vector<uint8_t>> NvsFlash::get_blob(const char *key) const {
  size_t size = this->get_blob(key, nullptr, 0);
  if (size != 0) {
    std::vector<uint8_t> res(size);
    if (this->get_blob(key, res.data(), res.size())) {
      return {res};
    }
  }
  return {};
}

bool NvsFlash::set_blob_(const char *key, const void *value, size_t size) const {
  if (!this->handle_) {
    return false;
  }
  auto err = this->handle_->set_blob(key, value, size);
  if (err != ESP_OK) {
    NVS_TRACE(TAG, "set blob %s failed: %s", key, esp_err_to_name(err));
    return false;
  }
  NVS_TRACE(TAG, "set blob %s success: %s", key, format_hex_pretty(static_cast<const uint8_t *>(value), size).c_str());
  return true;
}

optional<float> NvsFlash::get_float_(const char *key) const {
  if (!this->handle_) {
    return {};
  }
  union {
    uint32_t u;
    float f;
  } res{};
  auto err = this->handle_->get_item(key, res.u);
  if (err != ESP_OK) {
    NVS_TRACE(TAG, "get float %s failed: %s", key, esp_err_to_name(err));
    return {};
  }
  NVS_TRACE(TAG, "get float %s success: %f", key, res.f);
  return {res.f};
}

bool NvsFlash::set_float_(const char *key, float value) const {
  static_assert(sizeof(float) == sizeof(uint32_t));
  if (!this->handle_) {
    return false;
  }
  const auto *cv = static_cast<const uint32_t *>(static_cast<const void *>(&value));
  auto err = this->handle_->set_item(key, *cv);
  if (err != ESP_OK) {
    NVS_TRACE(TAG, "set float %s failed: %s", key, esp_err_to_name(err));
    return false;
  }
  NVS_TRACE(TAG, "set float %s success: %f", key, value);
  return true;
}

optional<double> NvsFlash::get_double_(const char *key) const {
  union {
    uint64_t u;
    double d;
  } res{};
  auto err = this->handle_->get_item(key, res.u);
  if (err != ESP_OK) {
    NVS_TRACE(TAG, "get double %s failed: %s", key, esp_err_to_name(err));
    return {};
  }
  NVS_TRACE(TAG, "get double %s success: %f", key, res.d);
  return {res.d};
}

bool NvsFlash::set_double_(const char *key, double value) const {
  static_assert(sizeof(double) == sizeof(uint64_t));
  if (!this->handle_) {
    return false;
  }
  const auto *cv = static_cast<const uint64_t *>(static_cast<const void *>(&value));
  auto err = this->handle_->set_item(key, *cv);
  if (err != ESP_OK) {
    NVS_TRACE(TAG, "set double %s failed: %s", key, esp_err_to_name(err));
    return false;
  }
  NVS_TRACE(TAG, "set double %s success: %f", key, value);
  return true;
}

optional<bool> NvsFlash::get_bool_(const char *key) const {
  if (!this->handle_) {
    return {};
  }
  uint8_t res{};
  auto err = this->handle_->get_item(key, res);
  if (err != ESP_OK) {
    NVS_TRACE(TAG, "get bool %s failed: %s", key, esp_err_to_name(err));
    return {};
  }
  NVS_TRACE(TAG, "get bool %s success: %s", key, TRUEFALSE(res != 0));
  return {res != 0};
}

bool NvsFlash::set_bool_(const char *key, bool value) const {
  if (!this->handle_) {
    return false;
  }
  const uint8_t cv = value ? 1 : 0;
  auto err = this->handle_->set_item(key, cv);
  if (err != ESP_OK) {
    NVS_TRACE(TAG, "set bool %s failed: %s", key, esp_err_to_name(err));
    return false;
  }
  NVS_TRACE(TAG, "set bool %s success: %s", key, TRUEFALSE(value));
  return true;
}

}  // namespace nvs_flash
}  // namespace esphome
