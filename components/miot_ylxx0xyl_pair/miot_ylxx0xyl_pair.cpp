#include "esphome/core/log.h"
#include "../miot/miot_dump.h"
#include "miot_ylxx0xyl_pair.h"

namespace esphome {
namespace miot_ylxx0xyl {

static const char *const TAG = "miot_ylxx0xyl_pair";

void MiotYLxx0xYLPair::dump_config() {
  ESP_LOGCONFIG(TAG, "Xiaomi product ID: %04X", this->product_id_);
  LOG_TEXT_SENSOR("  ", "Bindkey", this);
  LOG_TEXT_SENSOR("  ", "Version", this->version_);
}

static bool is_ylxx0xyl(uint16_t product_id) {
  switch (product_id) {
    case 0x0153:  // YLYK01YL          Remote
    case 0x03B6:  // YLKG07YL/YLKG08YL Dimmer
    case 0x03BF:  // YLYB01YL-BHFRC    Bathroom Heater Remote
    case 0x04E6:  // YLYK01YL-VENFAN   Fan Remote
    case 0x068E:  // YLYK01YL-FANCL    Ventilator Fan Remote
    case 0x15CE:  // MJGDYK01YL        Monitor Light Bar 1S Remote
      return true;
  }
  return false;
}

bool MiotYLxx0xYLPair::process_mibeacon(const miot::MiBeacon &mib) {
  if (this->product_id_ == 0 && is_ylxx0xyl(mib.product_id)) {
    this->product_id_ = mib.product_id;
  }
  if (this->product_id_ == mib.product_id && mib.has_object() && mib.object.id == miot::MIID_PAIRING_EVENT) {
    this->process_pairing_(mib.object);
    return true;
  }
  return false;
}

void MiotYLxx0xYLPair::process_pairing_(const miot::BLEObject &obj) {
  auto ble_client = this->auth_->parent();
  if (ble_client->state() == esp32_ble_tracker::ClientState::IDLE) {
    this->auth_->set_product_id(this->product_id_);
    ble_client->set_enabled(true);
  }
}

}  // namespace miot_ylxx0xyl
}  // namespace esphome
