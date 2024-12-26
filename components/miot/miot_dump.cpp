#include "esphome/core/log.h"
#include "inttypes.h"
#include "automation.h"

namespace esphome {
namespace miot {

void dump(const char *const TAG, const MiBeacon &mib) {
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
    auto mac = mac_reverse(mib.mac_address);
    ESP_LOGD(TAG, "  MAC address: " MIOT_ADDR_STR, MIOT_ADDR_HEX_REVERSE(mac));
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
    ESP_LOGD(TAG, "    ID  : %04" PRIX16, mib.object.id);
    ESP_LOGD(TAG, "    data: %s", format_hex_pretty(mib.object.data.data(), mib.object.data.size()).c_str());
  }
  if (mib.frame_control.is_encrypted) {
    ESP_LOGD(TAG, "  RND: %06" PRIX32, mib.random_number >> 8);
    ESP_LOGD(TAG, "  MIC: %08" PRIX32, mib.message_integrity_check);
  }
}

}  // namespace miot
}  // namespace esphome
