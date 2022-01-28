#pragma once

#include <stdint.h>

namespace esphome {
namespace miot_client {

#define FRAME_CTRL 0
#define MAGIC_NUM 0xA5
#define MAX_ATT_MTU 20

struct AuthFrame {
  enum Mode : uint8_t { MODE_CMD = 0, MODE_ACK = 1 };
  enum Cmd : uint8_t {
    CMD_PASS_THROUGH = 0x00,
    CMD_DEV_CERT = 0x01,
    CMD_DEV_MANU_CERT = 0x02,
    CMD_ECC_PUBKEY = 0x03,
    CMD_DEV_SIGNATURE = 0x04,
    CMD_DEV_LOGIN_INFO = 0x05,
    CMD_DEV_SHARE_INFO = 0x06,
    CMD_SERVER_CERT = 0x07,
    CMD_SERVER_SIGN = 0x08,
    CMD_MESH_CONFIG = 0x09,
    CMD_APP_CONFIRMATION = 0x0A,
    CMD_APP_RANDOM = 0x0B,
    CMD_DEV_CONFIRMATION = 0x0C,
    CMD_DEV_RANDOM = 0x0D,
    CMD_BIND_KEY = 0x0E,
  };
  enum Ack : uint8_t { ACK_SUCCESS = 0x00, ACK_READY, ACK_BUSY, ACK_TIMEOUT, ACK_CANCEL, ACK_LOST };
  uint16_t sn;
  union {
    uint8_t data[MAX_ATT_MTU - sizeof(sn)];
    struct {
      Mode mode;
      union {
        Cmd cmd;
        Ack ack;
      };
      uint16_t num;
    } ctrl;
  };

  bool is_cmd() const { return is_control() && ctrl.mode == MODE_CMD; }
  bool is_ack() const { return is_control() && ctrl.mode == MODE_ACK; }
  bool is_control() const { return sn == FRAME_CTRL; }
  static size_t data_size(size_t frame_size) { return frame_size - sizeof(sn); }
  static uint16_t frames_num(size_t data_size) { return (data_size + sizeof(data) - 1) / sizeof(data); }
};

enum OpCode : uint32_t {
  OPCODE_REG_TYPE = 0x10,
  OPCODE_REG_START = OPCODE_REG_TYPE + 0,
  OPCODE_REG_SUCCESS = OPCODE_REG_TYPE + 1,
  OPCODE_REG_FAILED = OPCODE_REG_TYPE + 2,
  OPCODE_REG_VERIFY_SUCC = OPCODE_REG_TYPE + 3,
  OPCODE_REG_VERIFY_FAIL = OPCODE_REG_TYPE + 4,
  OPCODE_REG_START_WO_PKI = OPCODE_REG_TYPE + 5,

  OPCODE_LOGIN_TYPE = 0x20,
  OPCODE_LOGIN_START = OPCODE_LOGIN_TYPE + 0,
  OPCODE_LOGIN_SUCCESS = OPCODE_LOGIN_TYPE + 1,
  OPCODE_LOGIN_INVALID_LTMK = OPCODE_LOGIN_TYPE + 2,
  OPCODE_LOGIN_FAILED = OPCODE_LOGIN_TYPE + 3,
  OPCODE_LOGIN_START_W_RANDOM = OPCODE_LOGIN_TYPE + 4,

  OPCODE_SYS_TYPE = 0xA0,
  // Read binding information
  OPCODE_SYS_KEY_RESTORE = OPCODE_SYS_TYPE,
  // Delete binding information
  OPCODE_SYS_KEY_DELETE = OPCODE_SYS_TYPE + 1,
  OPCODE_SYS_DEV_INFO_GET = OPCODE_SYS_TYPE + 2,
  OPCODE_SYS_UNKNOWN_A4 = OPCODE_SYS_TYPE + 4,

  OPCODE_ERR_TYPE = 0xE0UL,
  OPCODE_ERR_NOT_REGISTERED = OPCODE_ERR_TYPE + 1,
  OPCODE_ERR_REPEAT_LOGIN = OPCODE_ERR_TYPE + 2,
  OPCODE_ERR_INVALID_OOB = OPCODE_ERR_TYPE + 3,
};

}  // namespace miot_client
}  // namespace esphome
