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

void QingpingExplorer::dump_config() {
  ESP_LOGCONFIG(TAG, "Qingping Explorer:");
  this->dump_config_(TAG);
  ESP_LOGCONFIG(TAG, "  Max devices: %u", this->max_devices_);
}

bool QingpingExplorer::check_device_() {
  auto address = this->qp_->get_address();
  if (address == 0) {
    address = this->qp_->get_current_address();
  }
  this->address_ = address;

  for (auto device : this->devices_) {
    if (device == address) {
      return true;
    }
  }

  if (this->devices_.size() >= this->max_devices_) {
    if (!this->max_devices_warned_) {
      this->max_devices_warned_ = true;
      auto *mac = reinterpret_cast<const uint8_t *>(&address);
      ESP_LOGW(TAG, "Max devices %u reached, ignoring " MAC_FMT " and others", this->max_devices_,
               MAC_FMT_PARAMS(mac));
    }
    return false;
  }

  this->devices_.push_back(address);
  return true;
}

bool QingpingExplorer::update_(const QPDataPoint &dp) {
  if (!this->check_device_()) {
    return false;
  }

  switch (dp.type) {
    case QPID::TH: {
      auto *value = dp.cast<QPDataTH>();
      if (value == nullptr) {
        return false;
      }
      if (this->temperature_.name) {
        this->update_sensor_(this->temperature_, DEVICE_CLASS_TEMPERATURE, UNIT_CELSIUS, 1,
                             value->get_temperature());
      }
      if (this->humidity_.name) {
        this->update_sensor_(this->humidity_, DEVICE_CLASS_HUMIDITY, UNIT_PERCENT, 1, value->get_humidity());
      }
      break;
    }

    case QPID::BATTERY: {
      auto *value = dp.cast<QPDataBatteryLevel>();
      if (value == nullptr) {
        return false;
      }
      if (this->battery_level_.name) {
        this->update_sensor_(this->battery_level_, DEVICE_CLASS_BATTERY, UNIT_PERCENT, 0, value->get_battery_level());
      }
      break;
    }

    case QPID::DOOR: {
      auto *value = dp.cast<QPDataDoorState>();
      if (value == nullptr) {
        return false;
      }
      if (this->door_.name) {
        this->update_binary_sensor_(this->door_, DEVICE_CLASS_DOOR, value->get_door_state());
      }
      if (this->door_left_open_.name) {
        this->update_binary_sensor_(this->door_left_open_, DEVICE_CLASS_PROBLEM,
                                    value->get_door_left_open_problem());
      }
      break;
    }

    case QPID::PRESSURE: {
      auto *value = dp.cast<QPDataPressure>();
      if (value == nullptr) {
        return false;
      }
      if (this->pressure_.name) {
        this->update_sensor_(this->pressure_, DEVICE_CLASS_PRESSURE, UNIT_HECTOPASCAL, 1, value->get_pressure());
      }
      break;
    }

    case QPID::MOTION_ILLUMINANCE: {
      auto *value = dp.cast<QPDataMotionIlluminance>();
      if (value == nullptr) {
        return false;
      }
      if (this->motion_.name) {
        this->update_binary_sensor_(this->motion_, DEVICE_CLASS_MOTION, value->get_motion_state());
      }
      if (this->illuminance_.name) {
        this->update_sensor_(this->illuminance_, DEVICE_CLASS_ILLUMINANCE, UNIT_LUX, 0, value->get_illuminance());
      }
      break;
    }

    case QPID::ILLUMINANCE: {
      auto *value = dp.cast<QPDataIlluminance>();
      if (value == nullptr) {
        return false;
      }
      if (this->illuminance_.name) {
        this->update_sensor_(this->illuminance_, DEVICE_CLASS_ILLUMINANCE, UNIT_LUX, 0, value->get_illuminance());
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
      if (this->light_.name) {
        this->update_binary_sensor_(this->light_, DEVICE_CLASS_LIGHT, value->get_light_state());
      }
      break;
    }

    case QPID::PM: {
      auto *value = dp.cast<QPDataPM>();
      if (value == nullptr) {
        return false;
      }
      if (this->pm25_.name) {
        this->update_sensor_(this->pm25_, DEVICE_CLASS_PM25, UNIT_MICROGRAMS_PER_CUBIC_METER, 0,
                             value->get_pm25());
      }
      if (this->pm10_.name) {
        this->update_sensor_(this->pm10_, DEVICE_CLASS_PM10, UNIT_MICROGRAMS_PER_CUBIC_METER, 0,
                             value->get_pm10());
      }
      break;
    }

    case QPID::CO2: {
      auto *value = dp.cast<QPDataCO2>();
      if (value == nullptr) {
        return false;
      }
      if (this->co2_.name) {
        this->update_sensor_(this->co2_, DEVICE_CLASS_CARBON_DIOXIDE, UNIT_PARTS_PER_MILLION, 0, value->get_co2());
      }
      break;
    }

    default:
      return false;
  }
  return true;
}

void QingpingExplorer::update_binary_sensor_(const QPExplorerEntityConf &conf, const char *device_class, bool state) {
  auto obj_name = this->gen_obj_name(reinterpret_cast<const uint8_t *>(&this->address_), conf.name);
  auto obj_id = this->gen_obj_id(obj_name);

  for (auto *binary_sensor : this->binary_sensors_) {
    if (binary_sensor->get_qp_obj_id() == obj_id) {
      if (state != binary_sensor->state) {
        binary_sensor->publish_state(state);
      }
      return;
    }
  }

  ESP_LOGI(TAG, "New binary sensor: %s", obj_name.c_str());

  auto *binary_sensor = new QPExplorerBinarySensor(std::move(obj_name), std::move(obj_id), conf.fields);
  this->binary_sensors_.push_back(binary_sensor);

#if ESPHOME_VERSION_CODE < VERSION_CODE(2026, 3, 0)
  binary_sensor->set_device_class(device_class);
#endif

  App.register_binary_sensor(binary_sensor);
  binary_sensor->publish_state(state);
}

void QingpingExplorer::update_sensor_(const QPExplorerEntityConf &conf, const char *device_class, const char *uom,
                                      int8_t accuracy_decimals, float state) {
  auto obj_name = this->gen_obj_name(reinterpret_cast<const uint8_t *>(&this->address_), conf.name);
  auto obj_id = this->gen_obj_id(obj_name);

  for (auto *sensor : this->sensors_) {
    if (sensor->get_qp_obj_id() == obj_id) {
      if (sensor->get_force_update() || state != sensor->state) {
        sensor->publish_state(state);
      }
      return;
    }
  }

  ESP_LOGI(TAG, "New sensor: %s", obj_name.c_str());

  auto *sensor = new QPExplorerSensor(std::move(obj_name), std::move(obj_id), conf.fields);
  this->sensors_.push_back(sensor);

#if ESPHOME_VERSION_CODE < VERSION_CODE(2026, 3, 0)
  sensor->set_device_class(device_class);
  sensor->set_unit_of_measurement(uom);
#endif
  sensor->set_accuracy_decimals(accuracy_decimals);
  sensor->set_state_class(sensor::StateClass::STATE_CLASS_MEASUREMENT);

  App.register_sensor(sensor);
  sensor->publish_state(state);
}

}  // namespace qingping
}  // namespace esphome
#endif  // USE_QINGPING_EXPLORER
