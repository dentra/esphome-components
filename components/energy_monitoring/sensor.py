import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import sensor, time
from esphome.core import coroutine
from esphome.const import (
    CONF_ID,
    CONF_TIME_ID,
    CONF_CURRENT,
    CONF_POWER,
    CONF_VOLTAGE,
    CONF_POWER_FACTOR,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_POWER_FACTOR,
    ICON_FLASH,
    UNIT_VOLT_AMPS,
    UNIT_VOLT_AMPS_REACTIVE,
)

CODEOWNERS = ["@dentra"]

CONF_APPARENT_POWER = "apparent_power"
CONF_REACTIVE_POWER = "reactive_power"
CONF_WAIT_TIME = "wait_time"

UNIT_POWER_FACTOR = "Cos φ"

energy_monitoring_ns = cg.esphome_ns.namespace("energy_monitoring")

EnergyMonitoring = energy_monitoring_ns.class_("EnergyMonitoring", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(EnergyMonitoring),
        cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
        cv.Required(CONF_POWER): cv.use_id(sensor.Sensor),
        cv.Required(CONF_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_CURRENT): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_APPARENT_POWER): sensor.sensor_schema(
            UNIT_VOLT_AMPS, ICON_FLASH, 1, DEVICE_CLASS_EMPTY
        ),
        cv.Optional(CONF_REACTIVE_POWER): sensor.sensor_schema(
            UNIT_VOLT_AMPS_REACTIVE, ICON_FLASH, 1, DEVICE_CLASS_EMPTY
        ),
        cv.Optional(CONF_POWER_FACTOR): sensor.sensor_schema(
            UNIT_POWER_FACTOR, ICON_FLASH, 2, DEVICE_CLASS_POWER_FACTOR
        ),
        cv.Optional(CONF_WAIT_TIME, default=500): cv.uint32_t,
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

    # input sensors
    yield setup_input(config, CONF_POWER, var.set_power)
    yield setup_input(config, CONF_VOLTAGE, var.set_voltage)
    yield setup_input(config, CONF_CURRENT, var.set_current)

    # exposed sensors
    yield setup_sensor(config, CONF_REACTIVE_POWER, var.set_reactive_power)
    yield setup_sensor(config, CONF_POWER_FACTOR, var.set_power_factor)
    yield setup_sensor(config, CONF_APPARENT_POWER, var.set_apparent_power)

    # input options
    if CONF_WAIT_TIME in config:
        cg.add(var.set_wait_time(config[CONF_WAIT_TIME]))
