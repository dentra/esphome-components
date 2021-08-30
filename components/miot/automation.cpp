#ifdef ARDUINO_ARCH_ESP32

#include "esphome/core/log.h"
#include "inttypes.h"
#include "automation.h"

namespace esphome {
namespace miot {

static const char *TAG = "miot.automation";

static void dump(const MiBeacon &mib) {
  ESP_LOGD(TAG, "  Product ID: %04X", mib.product_id);
  ESP_LOGD(TAG, "  Frame Control:");
  ESP_LOGD(TAG, "    is_encrypted      : %s", YESNO(mib.frame_control.is_encrypted));
  ESP_LOGD(TAG, "    mac_include       : %s", YESNO(mib.frame_control.mac_include));
  ESP_LOGD(TAG, "    capability_include: %s", YESNO(mib.frame_control.capability_include));
  ESP_LOGD(TAG, "    object_include    : %s", YESNO(mib.frame_control.object_include));
  ESP_LOGD(TAG, "    mesh              : %s", YESNO(mib.frame_control.mesh));
  ESP_LOGD(TAG, "    registered        : %s", YESNO(mib.frame_control.registered));
  ESP_LOGD(TAG, "    solicited         : %s", YESNO(mib.frame_control.solicited));
  ESP_LOGD(TAG, "    auth_mode         : %" PRIu8, mib.frame_control.auth_mode);
  ESP_LOGD(TAG, "    version           : %" PRIu8, mib.frame_control.version);
  ESP_LOGD(TAG, "  Frame Counter: %" PRIu8, mib.frame_counter);
  if (mib.frame_control.mac_include) {
    const uint8_t *mac = reinterpret_cast<const uint8_t *>(&mib.mac_address);
    ESP_LOGD(TAG, "  MAC address: %02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  }
  if (mib.frame_control.capability_include) {
    ESP_LOGD(TAG, "  Capability:");
    ESP_LOGD(TAG, "    connectable : %s", YESNO(mib.capability.connectable));
    ESP_LOGD(TAG, "    centralable : %s", YESNO(mib.capability.centralable));
    ESP_LOGD(TAG, "    encryptable : %s", YESNO(mib.capability.encryptable));
    ESP_LOGD(TAG, "    bond_ability: %" PRIu8, mib.capability.bond_ability);
    ESP_LOGD(TAG, "    io          : %s", YESNO(mib.capability.io));
    ESP_LOGD(TAG, "    reserved    : %" PRIu8, mib.capability.reserved);
    if (mib.capability.io) {
      ESP_LOGD(TAG, "  IO Capability:");
      ESP_LOGD(TAG, "    can_enter_digits     : %s", YESNO(mib.io_capability.can_enter_digits));
      ESP_LOGD(TAG, "    can_enter_letters    : %s", YESNO(mib.io_capability.can_enter_letters));
      ESP_LOGD(TAG, "    can_read_nfc_tags    : %s", YESNO(mib.io_capability.can_read_nfc_tags));
      ESP_LOGD(TAG, "    can_recognize_qr_code: %s", YESNO(mib.io_capability.can_recognize_qr_code));
      ESP_LOGD(TAG, "    can_output_digits    : %s", YESNO(mib.io_capability.can_output_digits));
      ESP_LOGD(TAG, "    can_output_letters   : %s", YESNO(mib.io_capability.can_output_letters));
      ESP_LOGD(TAG, "    can_generate_nfc_tag : %s", YESNO(mib.io_capability.can_generate_nfc_tag));
      ESP_LOGD(TAG, "    can_generate_qr_code : %s", YESNO(mib.io_capability.can_generate_qr_code));
    }
  }
  if (mib.frame_control.object_include) {
    ESP_LOGD(TAG, "  Object:");
    ESP_LOGD(TAG, "    ID  : %04X", mib.object.id);
    ESP_LOGD(TAG, "    data: %s", hexencode(mib.object.data.data(), mib.object.data.size()).c_str());
  }
  if (mib.frame_control.is_encrypted) {
    ESP_LOGD(TAG, "  RND: %06X", mib.random_number >> 8);
    ESP_LOGD(TAG, "  MIC: %08X", mib.message_integrity_check);
  }
}

bool MiotAdvertiseTrigger::process_mibeacon(const MiBeacon &mib) {
  if (this->debug_) {
    dump(mib);
  }
  if (this->product_id_ != 0) {
    return MiotListener::process_mibeacon(mib);
  }
  this->product_id_ = mib.product_id;
  bool result = MiotListener::process_mibeacon(mib);
  this->product_id_ = 0;
  return result;
}

}  // namespace miot
}  // namespace esphome
#endif
