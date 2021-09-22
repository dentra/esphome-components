#include "esphome/core/log.h"
#include "miot_client.h"

namespace esphome {
namespace miot_client {

static const char *const TAG = "miot_client";

void MiotClient::dump_config() {
  ESP_LOGCONFIG(TAG, "Miot Client:");
  ESP_LOGCONFIG(TAG, "  Address: %s", this->address_str().c_str());
}

void MiotClient::setup() {
  ble_client::BLEClient::setup();
  this->enabled = false;
}

void MiotClient::loop() {
  if (this->enabled) {
    ble_client::BLEClient::loop();
  }
}

}  // namespace miot_client
}  // namespace esphome
