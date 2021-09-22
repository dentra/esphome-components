#pragma once

#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/ble_client/ble_client.h"

namespace esphome {
namespace miot_client {
class MiotClient : public ble_client::BLEClient {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;
};

}  // namespace miot_client
}  // namespace esphome
