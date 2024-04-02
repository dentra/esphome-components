#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/application.h"

#include "qingping.h"

namespace esphome {
namespace qingping {

static const char *const TAG = "qingping";

#define MAC_FMT "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_FMT_PARAMS(mac) mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]

bool Qingping::parse_device(const espbt::ESPBTDevice &device) {
  const auto address = device.address_uint64();
  for (const auto &service_data : device.get_service_datas()) {
    // 0000fdcd-0000-1000-8000-00805f9b34fb
    if (service_data.uuid.contains(0xCD, 0xFD)) {
      if (this->process_beacon_(address, service_data.data)) {
        return true;
      }
    }
    return false;
  }
  return false;
}

bool Qingping::process_beacon_(const uint64_t &address, const std::vector<uint8_t> &data) {
  if (this->verbose_) {
    auto *mac = reinterpret_cast<const uint8_t *>(&address);
    ESP_LOGD(TAG, "[" MAC_FMT "] Got beacon: %s", MAC_FMT_PARAMS(mac), format_hex_pretty(data).c_str());
  }

  if (data.size() < sizeof(QPBeaconHeader)) {
    auto *mac = reinterpret_cast<const uint8_t *>(&address);
    ESP_LOGW(TAG, "[" MAC_FMT "] Invalid beacon: %s", MAC_FMT_PARAMS(mac), format_hex_pretty(data).c_str());
    return false;
  }

  const auto *beacon = reinterpret_cast<const QPBeaconHeader *>(data.data());

  const auto &addr = this->use_beacon_mac_ ? beacon->mac.address : address;
  if (this->address_ != 0) {
    if (addr != this->address_) {
      return false;
    }
  } else {
    if (this->mac_exclude_.find(addr) != this->mac_exclude_.end()) {
      return false;
    }
  }

  const uint8_t *dp_data = data.data() + sizeof(QPBeaconHeader);
  size_t dp_size = data.size() - sizeof(QPBeaconHeader);

  if (this->verbose_) {
    ESP_LOGD(TAG, "  Frame control: %02X", beacon->raw_frame_control);
    ESP_LOGD(TAG, "     Product ID: %02X", beacon->product_id);
    ESP_LOGD(TAG, "    MAC-address: " MAC_FMT, MAC_FMT_PARAMS(beacon->mac.raw));
    ESP_LOGD(TAG, "           Data: %s", format_hex_pretty(dp_data, dp_size).c_str());
  }

  return this->process_dp_(dp_data, dp_size);
}

bool Qingping::process_dp_(const uint8_t *data, size_t size) {
  bool processed = false;
  while (size > 0) {
    const auto *dp = reinterpret_cast<const QPDataPoint *>(data);
    for (const auto &cb : this->callbacks_) {
      if (cb(*dp)) {
        processed = true;
      }
    }
    const auto dp_size = sizeof(QPDataPointHeader) + dp->size;
    size -= dp_size;
    data += dp_size;
  }
  return processed;
}

void Qingping::dump_config() {
  ESP_LOGCONFIG(TAG, "Qingping");
  auto *mac = this->get_mac();
  if (mac == nullptr) {
    ESP_LOGCONFIG(TAG, "  MAC-address: any");
  } else {
    ESP_LOGCONFIG(TAG, "  MAC-address: " MAC_FMT, MAC_FMT_PARAMS(mac));
  }
}

void QingpingComponent::dump_config_(const char *tag) {
  auto *mac = this->qp_->get_mac();
  if (mac) {
    ESP_LOGCONFIG(TAG, "  MAC-address: " MAC_FMT, MAC_FMT_PARAMS(mac));
  }
}

}  // namespace qingping
}  // namespace esphome
