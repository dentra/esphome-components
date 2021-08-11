import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ACTIVE_POWER,
    CONF_APPARENT_POWER,
    CONF_CURRENT,
    CONF_FREQUENCY,
    CONF_ID,
    CONF_POWER,
    CONF_POWER_FACTOR,
    CONF_REACTIVE_POWER,
    CONF_VOLTAGE,
    CONF_ENERGY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_POWER_FACTOR,
    DEVICE_CLASS_VOLTAGE,
    ICON_CURRENT_AC,
    ICON_EMPTY,
    LAST_RESET_TYPE_AUTO,
    STATE_CLASS_MEASUREMENT,
    UNIT_EMPTY,
    UNIT_HERTZ,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_VOLT_AMPS,
    UNIT_VOLT_AMPS_REACTIVE,
    UNIT_WATT,
    UNIT_WATT_HOURS,
)

DEPENDENCIES = ["uart"]

zmai90v1_ns = cg.esphome_ns.namespace("zmai90v1")
ZMAi90v1 = zmai90v1_ns.class_("ZMAi90v1", cg.PollingComponent, uart.UARTDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ZMAi90v1),
            cv.Optional(CONF_ENERGY): sensor.sensor_schema(
                UNIT_WATT_HOURS,
                ICON_EMPTY,
                0,
                DEVICE_CLASS_ENERGY,
                STATE_CLASS_MEASUREMENT,
                LAST_RESET_TYPE_AUTO,
            ),
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                UNIT_VOLT, ICON_EMPTY, 2, DEVICE_CLASS_VOLTAGE
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                UNIT_AMPERE,
                ICON_EMPTY,
                3,
                DEVICE_CLASS_CURRENT,
                STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ACTIVE_POWER): sensor.sensor_schema(
                UNIT_WATT, ICON_EMPTY, 2, DEVICE_CLASS_POWER, STATE_CLASS_MEASUREMENT
            ),
            cv.Optional(CONF_APPARENT_POWER): sensor.sensor_schema(
                UNIT_VOLT_AMPS,
                ICON_EMPTY,
                2,
                DEVICE_CLASS_POWER,
                STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_REACTIVE_POWER): sensor.sensor_schema(
                UNIT_VOLT_AMPS_REACTIVE,
                ICON_EMPTY,
                2,
                DEVICE_CLASS_POWER,
                STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FREQUENCY): sensor.sensor_schema(
                UNIT_HERTZ,
                ICON_CURRENT_AC,
                2,
                DEVICE_CLASS_EMPTY,
                STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_POWER_FACTOR): sensor.sensor_schema(
                UNIT_EMPTY,
                ICON_EMPTY,
                1,
                DEVICE_CLASS_POWER_FACTOR,
                STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_ENERGY in config:
        conf = config[CONF_ENERGY]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_energy(sens))
    if CONF_VOLTAGE in config:
        conf = config[CONF_VOLTAGE]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_voltage(sens))
    if CONF_CURRENT in config:
        conf = config[CONF_CURRENT]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_current(sens))
    if CONF_ACTIVE_POWER in config:
        conf = config[CONF_ACTIVE_POWER]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_active_power(sens))
    if CONF_APPARENT_POWER in config:
        conf = config[CONF_APPARENT_POWER]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_apparent_power(sens))
    if CONF_REACTIVE_POWER in config:
        conf = config[CONF_REACTIVE_POWER]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_reactive_power(sens))
    if CONF_FREQUENCY in config:
        conf = config[CONF_FREQUENCY]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_frequency(sens))
    if CONF_POWER_FACTOR in config:
        conf = config[CONF_POWER_FACTOR]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_power_factor(sens))
