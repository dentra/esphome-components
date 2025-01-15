import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, time
from esphome.const import (
    CONF_ID,
    CONF_POWER,
    CONF_TIME_ID,
    CONF_TOTAL,
    DEVICE_CLASS_ENERGY,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_KILOWATT_HOURS,
)

# Import custom ICONS from const.py
from .const import (
    ICON_TODAY,
    ICON_YESTERDAY,
    ICON_WEEK,
    ICON_MONTH,
    ICON_YEAR,
)

CODEOWNERS = ["@dentra"]

DEPENDENCIES = ["time"]

CONF_ENERGY_TODAY = "energy_today"
CONF_ENERGY_YESTERDAY = "energy_yesterday"
CONF_ENERGY_WEEK = "energy_week"
CONF_ENERGY_MONTH = "energy_month"
CONF_ENERGY_YEAR = "energy_year"

energy_statistics_ns = cg.esphome_ns.namespace("energy_statistics")

EnergyStatistics = energy_statistics_ns.class_("EnergyStatistics", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(EnergyStatistics),
        cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
        cv.Required(CONF_TOTAL): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_ENERGY_TODAY): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            icon=ICON_TODAY,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_ENERGY_YESTERDAY): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            icon=ICON_YESTERDAY,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_ENERGY_WEEK): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            icon=ICON_WEEK,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_ENERGY_MONTH): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            icon=ICON_MONTH,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_ENERGY_YEAR): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            icon=ICON_YEAR,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def setup_sensor(config, key, setter):
    """setting up sensor"""
    if key not in config:
        return None
    var = await sensor.new_sensor(config[key])
    cg.add(setter(var))
    return var


async def setup_input(config, key, setter):
    """setting up input"""
    if key not in config:
        return None
    var = await cg.get_variable(config[key])
    cg.add(setter(var))
    return var


# code generation entry point
async def to_code(config):
    """Code generation entry point"""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    await setup_input(config, CONF_TIME_ID, var.set_time)

    # input sensors
    await setup_input(config, CONF_POWER, var.set_power)
    await setup_input(config, CONF_TOTAL, var.set_total)

    # exposed sensors
    await setup_sensor(config, CONF_ENERGY_TODAY, var.set_energy_today)
    await setup_sensor(config, CONF_ENERGY_YESTERDAY, var.set_energy_yesterday)
    await setup_sensor(config, CONF_ENERGY_WEEK, var.set_energy_week)
    await setup_sensor(config, CONF_ENERGY_MONTH, var.set_energy_month)
    await setup_sensor(config, CONF_ENERGY_YEAR, var.set_energy_year)
