#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "../miot/miot.h"

namespace esphome {
namespace miot_lywsd03mmc {

class MiotLYWSD03MMC : public miot::MiotComponent, public sensor::Sensor {
 public:
  void dump_config() override;

  uint16_t get_product_id() const override { return 0x055B; }
  const char *get_product_code() const override { return "LYWSD03MMC"; }

  void set_humidity(sensor::Sensor *humidity) { this->humidity_ = humidity; }

 protected:
  sensor::Sensor *humidity_{nullptr};

  bool process_object_(const miot::BLEObject &obj) override;

  void process_temperature_(const miot::BLEObject &obj);
  void process_humidity_(const miot::BLEObject &obj);
};

}  // namespace miot_lywsd03mmc
}  // namespace esphome
