#pragma once
#include "esphome/components/ble_client/ble_client.h"
#include "miot_client.h"

namespace esphome {
namespace miot_client {

/**
 * Provide auth throught BT_UUID_MI_SECURE_AUTH.
 */
class MiotSecureAuthClient : public MiotClient, public AuthClient {
 public:
 protected:
};

}  // namespace miot_client
}  // namespace esphome
