import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import sensor, time
from esphome.const import (
    CONF_ID,
    CONF_TIME_ID,
    CONF_CURRENT,
    CONF_POWER,
    CONF_VOLTAGE,
    CONF_POWER_FACTOR,
    DEVICE_CLASS_POWER_FACTOR,
    ICON_FLASH,
    STATE_CLASS_MEASUREMENT,
    UNIT_VOLT_AMPS,
    UNIT_VOLT_AMPS_REACTIVE,
    UNIT_PERCENT,
)

CODEOWNERS = ["@dentra"]

CONF_APPARENT_POWER = "apparent_power"
CONF_REACTIVE_POWER = "reactive_power"
CONF_WAIT_TIME = "wait_time"

# UNIT_POWER_FACTOR = UNIT_PERCENT  # "Cos φ"

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
            unit_of_measurement=UNIT_VOLT_AMPS,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            icon=ICON_FLASH,
        ),
        cv.Optional(CONF_REACTIVE_POWER): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT_AMPS_REACTIVE,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            icon=ICON_FLASH,
        ),
        cv.Optional(CONF_POWER_FACTOR): sensor.sensor_schema(
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_POWER_FACTOR,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_WAIT_TIME, default=500): cv.uint32_t,
    }
).extend(cv.COMPONENT_SCHEMA)


async def setup_sensor(config, key, setter):
    if key not in config:
        return None
    var = await sensor.new_sensor(config[key])
    cg.add(setter(var))
    return var


async def setup_input(config, key, setter):
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

    # input sensors
    await setup_input(config, CONF_POWER, var.set_power)
    await setup_input(config, CONF_VOLTAGE, var.set_voltage)
    await setup_input(config, CONF_CURRENT, var.set_current)

    # exposed sensors
    await setup_sensor(config, CONF_REACTIVE_POWER, var.set_reactive_power)
    await setup_sensor(config, CONF_POWER_FACTOR, var.set_power_factor)
    await setup_sensor(config, CONF_APPARENT_POWER, var.set_apparent_power)

    # input options
    if CONF_WAIT_TIME in config:
        cg.add(var.set_wait_time(config[CONF_WAIT_TIME]))
