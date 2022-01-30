#include "esphome/core/log.h"
#include <array>
#include <inttypes.h>
#include "mbedtls/ccm.h"
#include "mbedtls/md.h"
#include "mbedtls/hkdf.h"
#include "miot_standard_auth_client.h"
#include "miot_client_crypto.h"

namespace esphome {
namespace miot_client {

static const char *const TAG = "miot_standard_auth_client";

void MiotStandardAuthClient::on_search_complete(const esp_ble_gattc_cb_param_t::gattc_search_cmpl_evt_param &param) {
  this->char_.ctrlp = this->get_char_handle(miot_client::BLE_UUID_MI_SERVICE, miot_client::BLE_UUID_MI_CONTROL_POINT);
  if (this->char_.ctrlp == ESP_GATT_ILLEGAL_HANDLE) {
    return;
  }
  this->char_.auth = this->get_char_handle(miot_client::BLE_UUID_MI_SERVICE, miot_client::BLE_UUID_MI_STANDARD_AUTH);
  if (this->char_.auth == ESP_GATT_ILLEGAL_HANDLE) {
    return;
  }

  this->char_.version = this->get_char_handle(miot_client::BLE_UUID_MI_SERVICE, miot_client::BLE_UUID_MI_VERSION);
  if (this->char_.version == ESP_GATT_ILLEGAL_HANDLE) {
    return;
  }

  if (this->load_ctx_ == nullptr) {
    this->load_ctx_ = new LoadCtx;
  }
  for (size_t i = 0; i < sizeof(this->load_ctx_->app_random_data); i++) {
    this->load_ctx_->app_random_data[i] = random_uint32();
  }
  this->register_for_notify(this->char_.ctrlp);
  this->register_for_notify(this->char_.auth);
}

void MiotStandardAuthClient::on_register_for_notification(
    const esp_ble_gattc_cb_param_t::gattc_reg_for_notify_evt_param &param) {
  if (param.handle == this->char_.ctrlp) {
    this->write_opcode_(OpCode::OPCODE_LOGIN_START_W_RANDOM);
  } else if (param.handle == this->char_.auth) {
    this->write_cmd_(AuthFrame::CMD_APP_RANDOM, sizeof(this->load_ctx_->app_random_data));
  }
}

void MiotStandardAuthClient::on_notify(const esp_ble_gattc_cb_param_t::gattc_notify_evt_param &param) {
  if (this->load_ctx_ == nullptr) {
    return;
  }
  if (param.handle == this->char_.ctrlp && param.value_len == sizeof(OpCode)) {
    this->process_ctrlp_(*reinterpret_cast<const OpCode *>(param.value));
  } else if (param.handle == this->char_.auth) {
    this->process_auth_(*reinterpret_cast<const AuthFrame *>(param.value), param.value_len);
  }
}

void MiotStandardAuthClient::process_ctrlp_(const OpCode opcode) {
  if (opcode == OpCode::OPCODE_LOGIN_SUCCESS) {
    ESP_LOGI(TAG, "Login Success");
    // success login
    this->node_state = esp32_ble_tracker::ClientState::ESTABLISHED;
    this->fire_auth_complete_();
    delete this->load_ctx_;
    this->load_ctx_ = nullptr;
  } else {
    ESP_LOGW(TAG, "Failed login: 0x%04X", opcode);
  }
}

static void debug(const char *prefix, const AuthFrame &frame, uint16_t frame_size) {
  if (frame.is_control()) {
    if (frame.is_cmd()) {
      ESP_LOGV(TAG, "%s CMD: 0x%02X (%u), num=%u", prefix, frame.ctrl.cmd, frame.ctrl.cmd, frame.ctrl.num);
    } else if (frame.is_ack()) {
      ESP_LOGV(TAG, "%s ACK: 0x%02X (%u)", prefix, frame.ctrl.ack, frame.ctrl.ack);
    } else {
      ESP_LOGV(TAG, "%s UNK: mode=0x%02X (%u)", prefix, frame.ctrl.mode, frame.ctrl.mode);
    }
  } else {
    ESP_LOGV(TAG, "%s DAT: sn=%u: %s", prefix, frame.sn,
             format_hex_pretty(frame.data, AuthFrame::data_size(frame_size)).c_str());
  }
}

void MiotStandardAuthClient::process_auth_(const AuthFrame &frame, uint16_t frame_size) {
  debug("<===", frame, frame_size);
  ESP_LOGD(TAG, "state=0x%X (%u)", this->state_, this->state_);

  if (this->state_ == AuthFrame::CMD_PASS_THROUGH && frame.is_cmd()) {
    this->state_ = frame.ctrl.cmd;
  }

  switch (this->state_) {
    case AuthFrame::CMD_APP_RANDOM:
      ESP_LOGV(TAG, "processing CMD_APP_RANDOM");
      this->process_auth_app_random_(frame, frame_size);
      break;
    case AuthFrame::CMD_DEV_RANDOM:
      ESP_LOGV(TAG, "processing CMD_DEV_RANDOM");
      this->process_auth_dev_random_(frame, frame_size);
      break;
    case AuthFrame::CMD_DEV_CONFIRMATION:
      ESP_LOGV(TAG, "processing CMD_DEV_CONFIRMATION");
      this->process_auth_dev_confirmation_(frame, frame_size);
      break;
    case AuthFrame::CMD_APP_CONFIRMATION:
      ESP_LOGV(TAG, "processing CMD_APP_CONFIRMATION");
      this->process_auth_app_confirmation_(frame, frame_size);
      break;
    default:
      ESP_LOGW(TAG, "Unknown state: 0x%X (%u)", this->state_, this->state_);
      break;
  }
}

void MiotStandardAuthClient::process_auth_app_random_(const AuthFrame &frame, uint16_t frame_size) {
  if (frame.is_ack()) {
    if (frame.ctrl.ack == AuthFrame::ACK_READY) {
      this->write_data_(this->load_ctx_->app_random_data, sizeof(this->load_ctx_->app_random_data));
    } else if (frame.ctrl.ack == AuthFrame::ACK_SUCCESS) {
      this->state_ = AuthFrame::CMD_PASS_THROUGH;
    } else {
      ESP_LOGE(TAG, "Awaiting ACK_READY or ACK_SUCCESS but got: 0x%02X", frame.ctrl.ack);
    }
  } else {
    ESP_LOGE(TAG, "Awaiting ACK but got: sn=%u, mode=0x%02X", frame.sn, frame.ctrl.mode);
  }
}

void MiotStandardAuthClient::process_auth_dev_random_(const AuthFrame &frame, uint16_t frame_size) {
  if (frame.is_control()) {
    if (frame.is_cmd()) {
      this->load_ctx_->max_rx_frames = frame.ctrl.num;
      this->write_ack_(AuthFrame::ACK_READY);
    } else {
      ESP_LOGE(TAG, "Awaiting CMD but not get it");
    }
    return;
  }

  const size_t data_size = AuthFrame::data_size(frame_size);

  ESP_LOGV(TAG, "Collect DEV_RANDOM %u of %u: %s", frame.sn, this->load_ctx_->max_rx_frames,
           format_hex_pretty(frame.data, data_size).c_str());

  // this->dev_random_data_.insert(this->dev_random_data_.end(), frame.data, frame.data + data_size);
  std::memcpy(this->load_ctx_->dev_random_data + (frame.sn - 1) * sizeof(frame.data), frame.data, data_size);

  if (frame.sn >= this->load_ctx_->max_rx_frames) {
    ESP_LOGV(TAG, "<=== dev_random_data: %s",
             format_hex_pretty(this->load_ctx_->dev_random_data, sizeof(this->load_ctx_->dev_random_data)).c_str());

    this->load_ctx_->max_rx_frames = 0;
    this->state_ = AuthFrame::CMD_PASS_THROUGH;
    this->generate_login_data_();
    this->write_ack_(AuthFrame::ACK_SUCCESS);
  }
}

void MiotStandardAuthClient::process_auth_app_confirmation_(const AuthFrame &frame, uint16_t frame_size) {
  if (frame.is_ack()) {
    if (frame.ctrl.ack == AuthFrame::ACK_READY) {
      this->write_data_(this->load_ctx_->app_confirmation_data, sizeof(this->load_ctx_->app_confirmation_data));
    } else if (frame.ctrl.ack == AuthFrame::ACK_SUCCESS) {
      this->state_ = AuthFrame::CMD_PASS_THROUGH;
    } else {
      ESP_LOGE(TAG, "Awaiting ACK_READY or ACK_SUCCESS but got: 0x%02X", frame.ctrl.ack);
    }
  } else {
    ESP_LOGE(TAG, "Awaiting ACK but got: sn=%u, mode=0x%02X", frame.sn, frame.ctrl.mode);
  }
}

void MiotStandardAuthClient::process_auth_dev_confirmation_(const AuthFrame &frame, uint16_t frame_size) {
  if (frame.is_control()) {
    if (frame.is_cmd()) {
      this->load_ctx_->max_rx_frames = frame.ctrl.num;
      this->write_ack_(AuthFrame::ACK_READY);
    } else {
      ESP_LOGE(TAG, "Awaiting CMD but not get it");
    }
    return;
  }

  const size_t data_size = AuthFrame::data_size(frame_size);

  ESP_LOGV(TAG, "Collect DEV_CONFIRMATION %u of %u: %s", frame.sn, this->load_ctx_->max_rx_frames,
           format_hex_pretty(frame.data, data_size).c_str());

  // this->dev_confirmation_data_.insert(this->dev_confirmation_data_.end(), frame.data, frame.data + data_size);
  std::memcpy(this->load_ctx_->dev_confirmation_data + (frame.sn - 1) * sizeof(frame.data), frame.data, data_size);

  if (frame.sn >= this->load_ctx_->max_rx_frames) {
    ESP_LOGV(TAG, "<=== dev_confirmation_data: %s",
             format_hex_pretty(this->load_ctx_->dev_confirmation_data, sizeof(this->load_ctx_->dev_confirmation_data))
                 .c_str());

    this->load_ctx_->max_rx_frames = 0;
    this->write_ack_(AuthFrame::ACK_SUCCESS);
    if (std::memcmp(this->load_ctx_->exp_confirmation_data, this->load_ctx_->dev_confirmation_data,
                    sizeof(this->load_ctx_->dev_confirmation_data)) != 0) {
      ESP_LOGW(TAG,
               "Fail compare\n"
               "  exp_confirmation_data: %s\n"
               "  dev_confirmation_data: %s\n",
               format_hex_pretty(this->load_ctx_->exp_confirmation_data, sizeof(this->load_ctx_->exp_confirmation_data))
                   .c_str(),
               format_hex_pretty(this->load_ctx_->dev_confirmation_data, sizeof(this->load_ctx_->dev_confirmation_data))
                   .c_str());
    }
    this->write_cmd_(AuthFrame::CMD_APP_CONFIRMATION, sizeof(this->load_ctx_->app_confirmation_data));
  }
}

void MiotStandardAuthClient::generate_login_data_() {
  uint8_t salt[sizeof(this->load_ctx_->app_random_data) + sizeof(this->load_ctx_->dev_random_data)];

  std::memcpy(salt + 0, this->load_ctx_->app_random_data, sizeof(this->load_ctx_->app_random_data));
  const auto n = sizeof(this->load_ctx_->app_random_data);
  std::memcpy(salt + n, this->load_ctx_->dev_random_data, sizeof(this->load_ctx_->dev_random_data));

  ESP_LOGV(TAG, "app_random_data: %s",
           format_hex_pretty(this->load_ctx_->app_random_data, sizeof(this->load_ctx_->app_random_data)).c_str());
  ESP_LOGV(TAG, "dev_random_data: %s",
           format_hex_pretty(this->load_ctx_->dev_random_data, sizeof(this->load_ctx_->dev_random_data)).c_str());
  ESP_LOGV(TAG, "salt1: %s", format_hex_pretty(salt, sizeof(salt)).c_str());

  auto md_info = mbedtls_md_info_from_type(mbedtls_md_type_t::MBEDTLS_MD_SHA256);

  const uint8_t info[] = {'m', 'i', 'b', 'l', 'e', '-', 'l', 'o', 'g', 'i', 'n', '-', 'i', 'n', 'f', 'o'};
  int res = mbedtls_hkdf(md_info, salt, sizeof(salt), this->token_, sizeof(this->token_), info, sizeof(info),
                         reinterpret_cast<uint8_t *>(&this->session_ctx_), sizeof(this->session_ctx_));
  if (res != 0) {
    ESP_LOGW(TAG, "mbedtls_hkdf failed: %d", res);
  }

  res = mbedtls_md_hmac(md_info, this->session_ctx_.app_key, sizeof(this->session_ctx_.app_key), salt, sizeof(salt),
                        this->load_ctx_->app_confirmation_data);
  if (res != 0) {
    ESP_LOGW(TAG, "mbedtls_md_hmac for app_confirmation_data failed: %d", res);
  }

  std::memcpy(salt + 0, this->load_ctx_->dev_random_data, sizeof(this->load_ctx_->dev_random_data));
  const auto m = sizeof(this->load_ctx_->dev_random_data);
  std::memcpy(salt + m, this->load_ctx_->app_random_data, sizeof(this->load_ctx_->app_random_data));
  ESP_LOGV(TAG, "salt2: %s", format_hex_pretty(salt, sizeof(salt)).c_str());

  res = mbedtls_md_hmac(md_info, this->session_ctx_.dev_key, sizeof(this->session_ctx_.dev_key), salt, sizeof(salt),
                        this->load_ctx_->exp_confirmation_data);
  if (res != 0) {
    ESP_LOGW(TAG, "mbedtls_md_hmac for exp_confirmation_data failed: %d", res);
  }

  ESP_LOGV(TAG, "prepared exp_confirmation_data: %s",
           format_hex_pretty(this->load_ctx_->exp_confirmation_data, sizeof(this->load_ctx_->exp_confirmation_data))
               .c_str());
  ESP_LOGV(TAG, "prepared app_confirmation_data: %s",
           format_hex_pretty(this->load_ctx_->app_confirmation_data, sizeof(this->load_ctx_->app_confirmation_data))
               .c_str());
}

bool MiotStandardAuthClient::write_data_(const uint8_t *data, const size_t data_size) {
  AuthFrame frame;
  const size_t max_frames = AuthFrame::frames_num(data_size);
  ESP_LOGV(TAG, "Prepare write_data: size=%u, frames num=%u", data_size, max_frames);
  for (size_t i = 0; i < max_frames; i++) {
    frame.sn = i + 1;

    size_t data_pos = i * sizeof(frame.data);
    size_t data_len = data_pos + sizeof(frame.data) < data_size ? sizeof(frame.data) : data_size - data_pos;
    std::memcpy(frame.data, data + data_pos, data_len);
    const uint16_t frame_size = sizeof(frame.sn) + data_len;
    debug("===>", frame, frame_size);
    if (!this->write_char(this->char_.auth, &frame, frame_size)) {
      return false;
    }
  }
  return true;
}

bool MiotStandardAuthClient::write_cmd_(AuthFrame::Cmd cmd, const size_t data_size) {
  AuthFrame frame;
  frame.sn = FRAME_CTRL;
  frame.ctrl.mode = AuthFrame::MODE_CMD;
  frame.ctrl.cmd = cmd;
  uint16_t frame_size = sizeof(frame.sn) + sizeof(frame.ctrl.mode) + sizeof(frame.ctrl.cmd);
  if (data_size > 0) {
    frame.ctrl.num = AuthFrame::frames_num(data_size);
    frame_size += sizeof(frame.ctrl.num);
  }
  debug("===>", frame, frame_size);
  this->state_ = cmd;
  return this->write_char(this->char_.auth, &frame, frame_size);
}

bool MiotStandardAuthClient::write_ack_(AuthFrame::Ack ack) {
  AuthFrame frame;
  frame.sn = FRAME_CTRL;
  frame.ctrl.mode = AuthFrame::MODE_ACK;
  frame.ctrl.ack = ack;
  uint16_t frame_size = sizeof(frame.sn) + sizeof(frame.ctrl.mode) + sizeof(frame.ctrl.ack);
  debug("===>", frame, frame_size);
  return this->write_char(this->char_.auth, &frame, frame_size);
}

void MiotStandardAuthClient::on_open(const esp_ble_gattc_cb_param_t::gattc_open_evt_param &param) {
  this->auth_seq_ = 0;
}

optional<std::vector<uint8_t>> MiotStandardAuthClient::decode(const uint8_t *data, const uint16_t size) const {
  ESP_LOGV(TAG, "Decoding %s", format_hex_pretty(data, size).c_str());

  // data struct: seq (2 bytes) + data (N bytes) + tag (4 bytes)
  auto nonce = SessionNonce(this->session_ctx_.dev_iv, *reinterpret_cast<const uint16_t *>(data));
  std::vector<uint8_t> res;
  const size_t tag_size = 4;
  res.resize(size - sizeof(uint16_t) - tag_size);
  auto input = data + sizeof(uint16_t);  // this->auth_inp_seq_
  auto tag = data + size - tag_size;
  int ret = session_decrypt(this->session_ctx_.dev_key, sizeof(this->session_ctx_.dev_key), nonce, input, res.size(),
                            res.data(), tag, tag_size);
  if (ret != 0) {
    ESP_LOGW(TAG, "session_decrypt failed: %d", ret);
    return {};
  }

  ESP_LOGV(TAG, "Decoded %s", format_hex_pretty(res.data(), res.size()).c_str());

  return res;
}

optional<std::vector<uint8_t>> MiotStandardAuthClient::encode(const uint8_t *data, const uint16_t size) {
  ESP_LOGV(TAG, "Encoding %s", format_hex_pretty(data, size).c_str());

  // buffer struct: seq (2 bytes) + data (N bytes) + tag (4 bytes)
  std::vector<uint8_t> res;
  const size_t tag_size = 4;
  res.resize(size + sizeof(this->auth_seq_) + tag_size);
  std::memcpy(res.data(), &this->auth_seq_, sizeof(this->auth_seq_));
  auto nonce = SessionNonce(this->session_ctx_.app_iv, this->auth_seq_);
  auto output = res.data() + sizeof(this->auth_seq_);
  auto tag = output + size;

  int ret = session_encrypt(this->session_ctx_.app_key, sizeof(this->session_ctx_.app_key), nonce, data, size, output,
                            tag, tag_size);
  if (ret != 0) {
    ESP_LOGW(TAG, "session_encrypt failed: %d", ret);
    return {};
  }

  ESP_LOGV(TAG, "Encoded %s", format_hex_pretty(res.data(), res.size()).c_str());

  this->auth_seq_++;

  return res;
}

}  // namespace miot_client
}  // namespace esphome
