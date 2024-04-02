#pragma once
#include "esphome/core/defines.h"
#ifdef USE_QINGPING_EXPLORER

#include <set>

#include "esphome/core/helpers.h"

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "qingping.h"

namespace esphome {
namespace qingping {

class QingpingExplorer : public QingpingComponent {
 public:
  QingpingExplorer(qingping::Qingping *qp) : QingpingComponent(qp) {}

  void dump_config() override;

  void setup() override {
    this->qp_->add_on_state_callback([this](const QPDataPoint &dp) { return this->update_(dp); });
  }

  void set_name_temperature(const char *value) { this->name_temperature_ = value; }
  void set_name_humidity(const char *value) { this->name_humidity_ = value; }
  void set_name_battery_level(const char *value) { this->name_battery_ = value; }
  void set_name_door(const char *value) { this->name_door_ = value; }
  void set_name_door_left_open(const char *value) { this->name_door_left_open_ = value; }
  void set_name_pressure(const char *value) { this->name_pressure_ = value; }
  void set_name_motion(const char *value) { this->name_motion_ = value; }
  void set_name_illuminance(const char *value) { this->name_illuminance_ = value; }
  void set_name_light(const char *value) { this->name_light_ = value; }
  void set_name_pm25(const char *value) { this->name_pm25_ = value; }
  void set_name_pm10(const char *value) { this->name_pm10_ = value; }
  void set_name_co2(const char *value) { this->name_co2_ = value; }

 protected:
  std::vector<sensor::Sensor *> sensors_;
  std::vector<binary_sensor::BinarySensor *> binary_sensors_;

  const char *name_temperature_{};
  const char *name_humidity_{};
  const char *name_battery_{};
  const char *name_door_{};
  const char *name_door_left_open_{};
  const char *name_pressure_{};
  const char *name_motion_{};
  const char *name_illuminance_{};
  const char *name_light_{};
  const char *name_pm25_{};
  const char *name_pm10_{};
  const char *name_co2_{};

  std::string gen_obj_name(const uint8_t *mac, const char *name) const {
    return mac ? str_sprintf("%s %02X%02X", name, mac[1], mac[0]) : std::string(name);
  }
  std::string gen_obj_id(const std::string &obj_name) const { return str_sanitize(str_snake_case(obj_name)); }

  bool update_(const QPDataPoint &dp);
  void update_binary_sensor_(const char *name, const char *device_class, bool state);
  void update_sensor_(const char *name, const char *device_class, const char *uom, int8_t accuracy_decimals,
                      float state);
};

}  // namespace qingping
}  // namespace esphome
#endif  // USE_QINGPING_EXPLORER
