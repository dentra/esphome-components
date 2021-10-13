#pragma once
#include "esphome/components/ble_client/ble_client.h"
#include "miot_client.h"

namespace esphome {
namespace miot_client {

/**
 * Provice auth throught BT_UUID_MI_STANDARD_AUTH.
 */
class MiotStandardAuthClient : public MiotClient, public AuthClient {
 public:
 protected:
};

}  // namespace miot_client
}  // namespace esphome
