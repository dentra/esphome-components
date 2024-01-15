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

optional<std::string> NvsFlash::get_string_(const char *key) const {
  if (!this->handle_) {
    return {};
  }

  size_t len{};
  esp_err_t err = this->handle_->get_item_size(nvs::ItemType::SZ, key, len);
  if (err != ESP_OK) {
    ESP_LOGV(TAG, "get_item_size %s: %s - the key might not be set yet", key, esp_err_to_name(err));
    return {};
  }

  std::string res;
  res.resize(len - 1);
  err = this->handle_->get_string(key, res.data(), len);
  if (err != ESP_OK) {
    ESP_LOGV(TAG, "get string %s failed: %s", key, esp_err_to_name(err));
    return {};
  }
  NVS_TRACE(TAG, "get string %s success: %s", key, res.c_str());
  return res;
}

optional<std::vector<uint8_t>> NvsFlash::get_blob_(const char *key) const {
  if (!this->handle_) {
    return {};
  }

  size_t len{};
  esp_err_t err = this->handle_->get_item_size(nvs::ItemType::BLOB, key, len);
  if (err != ESP_OK) {
    ESP_LOGV(TAG, "get_item_size %s: %s - the key might not be set yet", key, esp_err_to_name(err));
    return {};
  }

  std::vector<uint8_t> res;
  res.resize(len);
  err = this->handle_->get_blob(key, res.data(), len);
  if (err != ESP_OK) {
    ESP_LOGV(TAG, "get blob %s failed: %s", key, esp_err_to_name(err));
    return {};
  }
  res.resize(len);
  NVS_TRACE(TAG, "get blob %s success: %s", key, format_hex_pretty(res).c_str());
  return res;
}

}  // namespace nvs_flash
}  // namespace esphome
