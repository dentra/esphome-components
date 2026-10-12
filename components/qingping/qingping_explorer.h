#pragma once
#include "esphome/core/defines.h"
#ifdef USE_QINGPING_EXPLORER

#include <set>

#include "esphome/core/helpers.h"
#include "esphome/core/version.h"

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "qingping.h"

namespace esphome {
namespace qingping {

struct QPExplorerEntityConf {
  const char *name{};
  // Packed entity fields (device class and unit of measurement indexes), used since ESPHome 2026.3.0.
  uint32_t fields{};
};

template<class T> class QPExplorerEntity : public T {
 public:
  QPExplorerEntity(std::string &&name, std::string &&obj_id, uint32_t fields)
      : qp_name_(std::move(name)), qp_obj_id_(std::move(obj_id)) {
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2026, 3, 0)
    this->configure_entity_(this->qp_name_.c_str(), 0, fields);
#else
    this->set_name(this->qp_name_.c_str());
#if ESPHOME_VERSION_CODE < VERSION_CODE(2026, 1, 0)
    this->set_object_id(this->qp_obj_id_.c_str());
#endif
#endif
  }

  const std::string &get_qp_obj_id() const { return this->qp_obj_id_; }

 protected:
  std::string qp_name_;
  std::string qp_obj_id_;
};

using QPExplorerSensor = QPExplorerEntity<sensor::Sensor>;
using QPExplorerBinarySensor = QPExplorerEntity<binary_sensor::BinarySensor>;

class QingpingExplorer : public QingpingComponent {
 public:
  QingpingExplorer(qingping::Qingping *qp) : QingpingComponent(qp) {}

  void dump_config() override;

  void setup() override {
    this->qp_->add_on_state_callback([this](const QPDataPoint &dp) { return this->update_(dp); });
  }

  void set_name_temperature(const char *name, uint32_t fields = 0) { this->temperature_ = {name, fields}; }
  void set_name_humidity(const char *name, uint32_t fields = 0) { this->humidity_ = {name, fields}; }
  void set_name_battery_level(const char *name, uint32_t fields = 0) { this->battery_level_ = {name, fields}; }
  void set_name_door(const char *name, uint32_t fields = 0) { this->door_ = {name, fields}; }
  void set_name_door_left_open(const char *name, uint32_t fields = 0) { this->door_left_open_ = {name, fields}; }
  void set_name_pressure(const char *name, uint32_t fields = 0) { this->pressure_ = {name, fields}; }
  void set_name_motion(const char *name, uint32_t fields = 0) { this->motion_ = {name, fields}; }
  void set_name_illuminance(const char *name, uint32_t fields = 0) { this->illuminance_ = {name, fields}; }
  void set_name_light(const char *name, uint32_t fields = 0) { this->light_ = {name, fields}; }
  void set_name_pm25(const char *name, uint32_t fields = 0) { this->pm25_ = {name, fields}; }
  void set_name_pm10(const char *name, uint32_t fields = 0) { this->pm10_ = {name, fields}; }
  void set_name_co2(const char *name, uint32_t fields = 0) { this->co2_ = {name, fields}; }
  void set_max_devices(uint8_t max_devices) { this->max_devices_ = max_devices; }

 protected:
  std::vector<QPExplorerSensor *> sensors_;
  std::vector<QPExplorerBinarySensor *> binary_sensors_;

  QPExplorerEntityConf temperature_{};
  QPExplorerEntityConf humidity_{};
  QPExplorerEntityConf battery_level_{};
  QPExplorerEntityConf door_{};
  QPExplorerEntityConf door_left_open_{};
  QPExplorerEntityConf pressure_{};
  QPExplorerEntityConf motion_{};
  QPExplorerEntityConf illuminance_{};
  QPExplorerEntityConf light_{};
  QPExplorerEntityConf pm25_{};
  QPExplorerEntityConf pm10_{};
  QPExplorerEntityConf co2_{};

  uint8_t max_devices_{1};
  bool max_devices_warned_{};
  std::vector<uint64_t> devices_;
  // Address of the device being updated now.
  uint64_t address_{};

  bool check_device_();
  std::string gen_obj_name(const uint8_t *mac, const char *name) const {
    return mac ? str_sprintf("%s %02X%02X", name, mac[1], mac[0]) : std::string(name);
  }
  std::string gen_obj_id(const std::string &obj_name) const { return str_sanitize(str_snake_case(obj_name)); }

  bool update_(const QPDataPoint &dp);
  void update_binary_sensor_(const QPExplorerEntityConf &conf, const char *device_class, bool state);
  void update_sensor_(const QPExplorerEntityConf &conf, const char *device_class, const char *uom,
                      int8_t accuracy_decimals, float state);
};

}  // namespace qingping
}  // namespace esphome
#endif  // USE_QINGPING_EXPLORER
