#include "esphome/core/defines.h"
#ifdef USE_QINGPING_EXPLORER
#include "esphome/core/log.h"

#include "esphome/core/application.h"

#include "qingping_explorer.h"

namespace esphome {
namespace qingping {

static const char *const TAG = "qingping_auto";

static const char *const DEVICE_CLASS_CARBON_DIOXIDE = "carbon_dioxide";
static const char *const DEVICE_CLASS_PM10 = "pm10";
static const char *const DEVICE_CLASS_PM25 = "pm25";
static const char *const DEVICE_CLASS_TEMPERATURE = "temperature";
static const char *const DEVICE_CLASS_HUMIDITY = "humidity";
static const char *const DEVICE_CLASS_BATTERY = "battery";

static const char *const DEVICE_CLASS_PRESSURE = "pressure";
static const char *const DEVICE_CLASS_ILLUMINANCE = "illuminance";

static const char *const DEVICE_CLASS_DOOR = "door";
static const char *const DEVICE_CLASS_PROBLEM = "problem";
static const char *const DEVICE_CLASS_MOTION = "motion";
static const char *const DEVICE_CLASS_LIGHT = "light";

static const char *const UNIT_PARTS_PER_MILLION = "ppm";
static const char *const UNIT_MICROGRAMS_PER_CUBIC_METER = "µg/m³";
static const char *const UNIT_CELSIUS = "°C";
static const char *const UNIT_PERCENT = "%";
static const char *const UNIT_LUX = "lx";
static const char *const UNIT_HECTOPASCAL = "hPa";

template<class T> class QPExplorerEntity : public T {
 public:
  explicit QPExplorerEntity(std::string &&name, std::string &&obj_id)
      : qp_name_(std::move(name)), qp_obj_id_(std::move(obj_id)) {
    this->set_name(this->qp_name_.c_str());
    this->set_object_id(this->qp_obj_id_.c_str());
  }

 protected:
  std::string qp_name_;
  std::string qp_obj_id_;
};

using QPExplorerSensor = QPExplorerEntity<sensor::Sensor>;
using QPExplorerBinarySensor = QPExplorerEntity<binary_sensor::BinarySensor>;

void QingpingExplorer::dump_config() {
  ESP_LOGCONFIG(TAG, "Qingping Explorer:");
  this->dump_config_(TAG);
}

bool QingpingExplorer::update_(const QPDataPoint &dp) {
  switch (dp.type) {
    case QPID::TH: {
      auto *value = dp.cast<QPDataTH>();
      if (value == nullptr) {
        return false;
      }
      if (this->name_temperature_) {
        this->update_sensor_(this->name_temperature_, DEVICE_CLASS_TEMPERATURE, UNIT_CELSIUS, 1,
                             value->get_temperature());
      }
      if (this->name_humidity_) {
        this->update_sensor_(this->name_humidity_, DEVICE_CLASS_HUMIDITY, UNIT_PERCENT, 1, value->get_humidity());
      }
      break;
    }

    case QPID::BATTERY: {
      auto *value = dp.cast<QPDataBatteryLevel>();
      if (value == nullptr) {
        return false;
      }
      if (this->name_battery_) {
        this->update_binary_sensor_(this->name_battery_, DEVICE_CLASS_BATTERY, value->get_battery_level());
      }
      break;
    }

    case QPID::DOOR: {
      auto *value = dp.cast<QPDataDoorState>();
      if (value == nullptr) {
        return false;
      }
      if (this->name_door_) {
        this->update_binary_sensor_(this->name_door_, DEVICE_CLASS_DOOR, value->get_door_state());
      }
      if (this->name_door_left_open_) {
        this->update_binary_sensor_(this->name_door_left_open_, DEVICE_CLASS_PROBLEM,
                                    value->get_door_left_open_problem());
      }
      break;
    }

    case QPID::PRESSURE: {
      auto *value = dp.cast<QPDataPressure>();
      if (value == nullptr) {
        return false;
      }
      if (this->name_pressure_) {
        this->update_sensor_(this->name_pressure_, DEVICE_CLASS_PRESSURE, UNIT_HECTOPASCAL, 1, value->get_pressure());
      }
      break;
    }

    case QPID::MOTION_ILLUMINANCE: {
      auto *value = dp.cast<QPDataMotionIlluminance>();
      if (value == nullptr) {
        return false;
      }
      if (this->name_motion_) {
        this->update_binary_sensor_(this->name_motion_, DEVICE_CLASS_MOTION, value->get_motion_state());
      }
      if (this->name_illuminance_) {
        this->update_sensor_(this->name_illuminance_, DEVICE_CLASS_ILLUMINANCE, UNIT_LUX, 0, value->get_illuminance());
      }
      break;
    }

    case QPID::ILLUMINANCE: {
      auto *value = dp.cast<QPDataIlluminance>();
      if (value == nullptr) {
        return false;
      }
      if (this->name_illuminance_) {
        this->update_sensor_(this->name_illuminance_, DEVICE_CLASS_ILLUMINANCE, UNIT_LUX, 0, value->get_illuminance());
      }
      break;
    }

    case QPID::PACKET: {
      // nothing to do
      break;
    }

    case QPID::LIGHT: {
      auto *value = dp.cast<QPDataLightState>();
      if (value == nullptr) {
        return false;
      }
      if (this->name_light_) {
        this->update_binary_sensor_(this->name_light_, DEVICE_CLASS_LIGHT, value->get_light_state());
      }
      break;
    }

    case QPID::PM: {
      auto *value = dp.cast<QPDataPM>();
      if (value == nullptr) {
        return false;
      }
      if (this->name_pm25_) {
        this->update_sensor_(this->name_pm25_, DEVICE_CLASS_PM25, UNIT_MICROGRAMS_PER_CUBIC_METER, 0,
                             value->get_pm25());
      }
      if (this->name_pm10_) {
        this->update_sensor_(this->name_pm10_, DEVICE_CLASS_PM10, UNIT_MICROGRAMS_PER_CUBIC_METER, 0,
                             value->get_pm10());
      }
      break;
    }

    case QPID::CO2: {
      auto *value = dp.cast<QPDataCO2>();
      if (value == nullptr) {
        return false;
      }
      if (this->name_co2_) {
        this->update_sensor_(this->name_co2_, DEVICE_CLASS_CARBON_DIOXIDE, UNIT_PARTS_PER_MILLION, 0, value->get_co2());
      }
      break;
    }

    default:
      return false;
  }
  return true;
}

void QingpingExplorer::update_binary_sensor_(const char *name, const char *device_class, bool state) {
  auto obj_name = this->gen_obj_name(this->qp_->get_mac(), name);
  auto obj_id = this->gen_obj_id(obj_name);

  for (auto *binary_sensor : this->binary_sensors_) {
    if (binary_sensor->get_object_id() == obj_id) {
      if (state != binary_sensor->state) {
        binary_sensor->publish_state(state);
      }
      return;
    }
  }

  ESP_LOGI(TAG, "New binary sensor: %s", obj_name.c_str());

  auto *binary_sensor = new QPExplorerBinarySensor(std::move(obj_name), std::move(obj_id));
  this->binary_sensors_.push_back(binary_sensor);

  binary_sensor->set_device_class(device_class);

  App.register_binary_sensor(binary_sensor);
  binary_sensor->publish_state(state);
}

void QingpingExplorer::update_sensor_(const char *name, const char *device_class, const char *uom,
                                      int8_t accuracy_decimals, float state) {
  auto obj_name = this->gen_obj_name(this->qp_->get_mac(), name);
  auto obj_id = this->gen_obj_id(obj_name);

  for (auto *sensor : this->sensors_) {
    if (sensor->get_object_id() == obj_id) {
      if (sensor->get_force_update() || state != sensor->state) {
        sensor->publish_state(state);
      }
      return;
    }
  }

  ESP_LOGI(TAG, "New sensor: %s", obj_name.c_str());

  auto *sensor = new QPExplorerSensor(std::move(obj_name), std::move(obj_id));
  this->sensors_.push_back(sensor);

  sensor->set_device_class(device_class);
  sensor->set_unit_of_measurement(uom);
  sensor->set_accuracy_decimals(accuracy_decimals);
  sensor->set_state_class(sensor::StateClass::STATE_CLASS_MEASUREMENT);

  App.register_sensor(sensor);
  sensor->publish_state(state);
}

}  // namespace qingping
}  // namespace esphome
#endif  // USE_QINGPING_EXPLORER
