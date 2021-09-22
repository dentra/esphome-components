#include "esphome/core/log.h"
#include "miot_ylxx0xyl_pair.h"
#include "cipher.h"

namespace esphome {
namespace miot_ylxx0xyl_pair {

static const char *const TAG = "miot_ylxx0xyl_pair";

// authInitCharacteristic (authentication)
constexpr uint16_t CHAR_AUTH = 0x0010;
// authCharacteristic (token)
constexpr uint16_t CHAR_TOKEN = 0x0001;
// verCharacteristics (ver)
constexpr uint16_t CHAR_VER = 0x0004;
// (beakonkey)
constexpr uint16_t CHAR_BEACONKEY = 0x0014;

constexpr uint8_t MI_KEY1[] = {0x90, 0xCA, 0x85, 0xDE};
constexpr uint8_t MI_KEY2[] = {0x92, 0xAB, 0x54, 0xFA};

void MiotYLxx0xYLPairNode::on_disconnect(const esp_ble_gattc_cb_param_t::gattc_disconnect_evt_param &param) {
  this->characteristics_ = {};
  if (this->progress_ != NONE) {
    ESP_LOGW(TAG, "Pairning process aborted due to device disconnect");
    this->pair_->publish_state("Pairning process aborted due to device disconnect");
    this->progress_ = NONE;
  }
  // disable auto reconnect
  this->parent()->set_enabled(false);
}

void MiotYLxx0xYLPairNode::on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) {
  auto service = this->get_miot_service();
  if (service == nullptr) {
    ESP_LOGW(TAG, "No MIOT service UUID found");
    return;
  }

  auto auth = service->get_characteristic(CHAR_AUTH);
  if (auth) {
    this->characteristics_.init = auth->handle;
    ESP_LOGD(TAG, "authInitCharacteristic %04X (%u)", this->characteristics_.init, this->characteristics_.init);
  } else {
    ESP_LOGE(TAG, "Can't discover characteristics %04X", CHAR_AUTH);
    return;
  }

  auto token = service->get_characteristic(CHAR_TOKEN);
  if (token) {
    this->characteristics_.auth = token->handle;
    ESP_LOGD(TAG, "authCharacteristic %04X (%u)", this->characteristics_.auth, this->characteristics_.auth);
  } else {
    ESP_LOGE(TAG, "Can't discover characteristics %04X", CHAR_TOKEN);
    return;
  }

  auto ver = service->get_characteristic(CHAR_VER);
  if (ver) {
    this->characteristics_.ver = ver->handle;
    ESP_LOGD(TAG, "verCharacteristics %04X (%u)", this->characteristics_.ver, this->characteristics_.ver);
  } else {
    ESP_LOGE(TAG, "Can't discover characteristics %04X", CHAR_VER);
    return;
  }

  auto key = service->get_characteristic(CHAR_BEACONKEY);
  if (key) {
    this->characteristics_.key = key->handle;
    ESP_LOGD(TAG, "keyCharacteristics %04X (%u)", this->characteristics_.key, this->characteristics_.key);
  } else {
    ESP_LOGE(TAG, "Can't discover characteristics %04X", CHAR_BEACONKEY);
    return;
  }

  this->node_state = esp32_ble_tracker::ClientState::Established;

  this->start_pairing();
}

void MiotYLxx0xYLPairNode::on_write_char(const esp_ble_gattc_cb_param_t::gattc_write_evt_param &param) {
  ESP_LOGI(TAG, "write_char 0x%x is written, status=0x%x", param.handle, param.status);
  if (this->progress_ == STEP1 && param.handle == this->characteristics_.init) {
    ESP_LOGI(TAG, "step %d complete: %s", STEP1, YESNO(param.status == ESP_GATT_OK));
    this->step2_();
  } else if (this->progress_ == STEP5 && param.handle == this->characteristics_.auth) {
    ESP_LOGI(TAG, "step %d complete: %s", STEP5, YESNO(param.status == ESP_GATT_OK));
    this->step6_();
  } else if (this->progress_ == STEP3 && param.handle == this->characteristics_.auth) {
    ESP_LOGI(TAG, "step %d complete: %s", STEP3, YESNO(param.status == ESP_GATT_OK));
  }
}

void MiotYLxx0xYLPairNode::on_register_for_notification(
    const esp_ble_gattc_cb_param_t::gattc_reg_for_notify_evt_param &param) {
  ESP_LOGI(TAG, "notification 0x%x registered, status=0x%x", param.handle, param.status);
  if (this->progress_ == STEP2 && param.handle == this->characteristics_.auth) {
    ESP_LOGI(TAG, "step %d complete: %s", STEP2, YESNO(param.status == ESP_GATT_OK));
    this->step3_();
  }
}

void MiotYLxx0xYLPairNode::on_read_char(const esp_ble_gattc_cb_param_t::gattc_read_char_evt_param &param) {
  ESP_LOGI(TAG, "got read_char for 0x%x, data: %s", param.handle, hexencode(param.value, param.value_len).c_str());
  if (param.handle == this->characteristics_.ver) {
    this->on_read_ver_(param.value, param.value_len);
  } else if (param.handle == this->characteristics_.key) {
    this->on_read_key_(param.value, param.value_len);
  }
}

void MiotYLxx0xYLPairNode::on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) {
  ESP_LOGI(TAG, "got notification for 0x%x, data: %s", param.handle, hexencode(param.value, param.value_len).c_str());

  // step 4
  if (this->progress_ == STEP3 && param.handle == this->characteristics_.auth) {
    const auto reversed_mac = (uint8_t *) &this->parent()->address;
    const auto product_id = this->pair_->get_product_id();
    const auto result =
        cipher::cipher(cipher::mix_b(reversed_mac, product_id),
                       cipher::cipher(cipher::mix_a(reversed_mac, product_id), param.value, param.value_len));

    const bool complete =
        this->token_.size() == result.size() && memcmp(this->token_.data(), result.data(), result.size()) == 0;
    ESP_LOGI(TAG, "step %d complete: %s", STEP4, YESNO(complete));

    if (!complete) {
      this->pair_->publish_state("Pairing process failed at step 4");
    } else {
      this->step5_();
    }
  }
}

void MiotYLxx0xYLPairNode::on_read_ver_(const uint8_t *data, int size) {
  auto res = cipher::cipher(this->token_, data, size);

  this->pair_->publish_version(std::string(reinterpret_cast<char const *>(res.data()), res.size()));

  this->parent()->set_enabled(false);
  this->pair_->publish_state("Pairing process success");
  this->progress_ = NONE;
}

void MiotYLxx0xYLPairNode::on_read_key_(const uint8_t *data, int size) {
  auto res = cipher::cipher(this->token_, data, size);
  char buf[4];
  std::string s;
  for (int b : res) {
    sprintf(buf, "%02X", b);
    s += buf;
  }
  this->pair_->publish_bindkey(s);
}

void MiotYLxx0xYLPairNode::start_pairing() {
  if (this->progress_ != NONE) {
    ESP_LOGW(TAG, "Pairing process already started");
    return;
  }

  ESP_LOGI(TAG, "Pairing process started");
  this->token_ = cipher::generate_random_token();
  ESP_LOGD(TAG, "Using token: %s", hexencode(this->token_).c_str());

  this->step1_();
}

void MiotYLxx0xYLPairNode::step1_() {
  this->progress_ = STEP1;
  this->pair_->publish_state("Pairing process step 1");

  this->writeCharacteristic(this->characteristics_.init, MI_KEY1, sizeof(MI_KEY1), true);
}

void MiotYLxx0xYLPairNode::step2_() {
  this->progress_ = STEP2;
  this->pair_->publish_state("Pairing process step 2");

  this->subscribeCharacteristic(this->characteristics_.auth);
}

void MiotYLxx0xYLPairNode::step3_() {
  this->progress_ = STEP3;
  this->pair_->publish_state("Pairing process step 3");

  const auto reversed_mac = (uint8_t *) &this->parent()->address;
  auto step3 = cipher::cipher(cipher::mix_a(reversed_mac, this->pair_->get_product_id()), this->token_);
  this->writeCharacteristic(this->characteristics_.auth, step3, true);
}

void MiotYLxx0xYLPairNode::step5_() {
  this->progress_ = STEP5;
  this->pair_->publish_state("Pairing process step 5");

  this->writeCharacteristic(this->characteristics_.auth, cipher::cipher(this->token_, MI_KEY2, sizeof(MI_KEY2)), true);
}

void MiotYLxx0xYLPairNode::step6_() {
  // get the beaconkey
  this->readCharacteristic(this->characteristics_.key);

  this->progress_ = STEP6;
  this->pair_->publish_state("Pairing process step 6");

  this->readCharacteristic(this->characteristics_.ver);
}

void MiotYLxx0xYLPair::dump_config() {
  this->dump_config_(TAG);
  ESP_LOGCONFIG(TAG, "  Product ID: %04X", this->product_id_);
  LOG_TEXT_SENSOR("  ", "Status", this);
  LOG_TEXT_SENSOR("  ", "Bindkey", this->bindkey_);
  LOG_TEXT_SENSOR("  ", "Version", this->version_);
}

static bool is_ylxx0xyl(uint16_t product_id) {
  switch (product_id) {
    case 0x0153:  // YLYK01YL          Remote
    case 0x03B6:  // YLKG07YL/YLKG08YL Dimmer
    case 0x03BF:  // YLYB01YL-BHFRC    Bathroom Heater Remote
    case 0x04E6:  // YLYK01YL-VENFAN   Fan Remote
    case 0x068E:  // YLYK01YL-FANCL    Ventilator Fan Remote
      return true;
  }
  return false;
}

bool MiotYLxx0xYLPair::process_mibeacon(const miot::MiBeacon &mib) {
  if (this->product_id_ == 0 && is_ylxx0xyl(mib.product_id)) {
    this->product_id_ = mib.product_id;
    // char tmp[6] = {};
    // sprintf(tmp, "%04X", this->product_id_);
    // this->publish_state(tmp);
  }
  if (mib.has_object() && mib.object.id == miot::MIID_SIMPLE_PAIRING_EVENT) {
    // pairing event always contains frame counter with the same value (1), so reset them
    this->frame_counter_ = 0;
  }
  return MiotListener::process_mibeacon(mib);
}

void MiotYLxx0xYLPair::process_pairing_(const miot::BLEObject &obj) {
  ESP_LOGI(TAG, "Pairing object %04X", *obj.get_pairing_object());
  if (this->client_.state() == esp32_ble_tracker::ClientState::Idle) {
    this->publish_state("Starting pairing process");
    this->client_.set_enabled(true);
  }
}

bool MiotYLxx0xYLPair::process_object_(const miot::BLEObject &obj) {
  switch (obj.id) {
    case miot::MIID_SIMPLE_PAIRING_EVENT:
      this->process_pairing_(obj);
      break;
    default:
      return this->process_default_(obj);
  }
  return true;
}

}  // namespace miot_ylxx0xyl_pair
}  // namespace esphome
