#pragma once
#include "esphome/components/ble_client/ble_client.h"
#include "miot_client.h"

namespace esphome {
namespace miot_client {

/**
 * Provide auth throught BLE_UUID::MI_SECURE_AUTH.
 * https://github.com/Ai-Thinker-Open/Telink_SIG_Mesh/blob/master/example/CW_Ali_Mesh/mesh/mi_api/certi/secure_auth/mible_secure_auth.c
 */
class MiotSecureAuthClient : public MiotClient, public AuthClient {
 public:
 protected:
};

}  // namespace miot_client
}  // namespace esphome
