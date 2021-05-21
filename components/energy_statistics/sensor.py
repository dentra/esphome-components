import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import sensor, time
from esphome.core import coroutine
from esphome.const import (
    CONF_ID,
    CONF_TIME_ID,
    CONF_POWER,
    CONF_TOTAL,
    DEVICE_CLASS_ENERGY,
    ICON_FLASH,
)

CODEOWNERS = ["@dentra"]

DEPENDENCIES = ["time"]

CONF_ENERGY_TODAY = "energy_today"
CONF_ENERGY_YESTERDAY = "energy_yesterday"
CONF_ENERGY_WEEK = "energy_week"
CONF_ENERGY_MONTH = "energy_month"

UNIT_KWATT_HOURS = "kWh"

energy_statistics_ns = cg.esphome_ns.namespace("energy_statistics")

EnergyStatistics = energy_statistics_ns.class_("EnergyStatistics", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(EnergyStatistics),
        cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
        cv.Required(CONF_TOTAL): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_ENERGY_TODAY): sensor.sensor_schema(
            UNIT_KWATT_HOURS, ICON_FLASH, 2, DEVICE_CLASS_ENERGY
        ),
        cv.Optional(CONF_ENERGY_YESTERDAY): sensor.sensor_schema(
            UNIT_KWATT_HOURS, ICON_FLASH, 2, DEVICE_CLASS_ENERGY
        ),
        cv.Optional(CONF_ENERGY_WEEK): sensor.sensor_schema(
            UNIT_KWATT_HOURS, ICON_FLASH, 2, DEVICE_CLASS_ENERGY
        ),
        cv.Optional(CONF_ENERGY_MONTH): sensor.sensor_schema(
            UNIT_KWATT_HOURS, ICON_FLASH, 2, DEVICE_CLASS_ENERGY
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


@coroutine
def setup_sensor(config, key, setter):
    if key not in config:
        return None
    var = yield sensor.new_sensor(config[key])
    cg.add(setter(var))
    return var


@coroutine
def setup_input(config, key, setter):
    if key not in config:
        return None
    var = yield cg.get_variable(config[key])
    cg.add(setter(var))
    return var


# code generation entry point
def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    yield setup_input(config, CONF_TIME_ID, var.set_time)

    # input sensors
    yield setup_input(config, CONF_POWER, var.set_power)
    yield setup_input(config, CONF_TOTAL, var.set_total)

    # exposed sensors
    yield setup_sensor(config, CONF_ENERGY_TODAY, var.set_energy_today)
    yield setup_sensor(config, CONF_ENERGY_YESTERDAY, var.set_energy_yesterday)
    yield setup_sensor(config, CONF_ENERGY_WEEK, var.set_energy_week)
    yield setup_sensor(config, CONF_ENERGY_MONTH, var.set_energy_month)
