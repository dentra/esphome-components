import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, sensor
from esphome.const import (
    CONF_IDLE_TIME,
    CONF_ILLUMINANCE,
    CONF_LIGHT,
    CONF_NAME,
    CONF_TIMEOUT,
    DEVICE_CLASS_ILLUMINANCE,
    DEVICE_CLASS_LIGHT,
    DEVICE_CLASS_MOTION,
    ICON_TIMELAPSE,
    STATE_CLASS_MEASUREMENT,
    UNIT_LUX,
    UNIT_SECOND,
)

from .. import miot  # pylint: disable=relative-beyond-top-level

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot", "sensor"]

miot_motion = cg.esphome_ns.namespace("miot_motion")
MiotMotion = miot_motion.class_(
    "MiotMotion", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(MiotMotion, device_class=DEVICE_CLASS_MOTION)
    .extend(
        {
            cv.Optional(CONF_LIGHT): cv.maybe_simple_value(
                binary_sensor.binary_sensor_schema(
                    device_class=DEVICE_CLASS_LIGHT,
                ),
                key=CONF_NAME,
            ),
            cv.Optional(CONF_IDLE_TIME): cv.maybe_simple_value(
                sensor.sensor_schema(
                    unit_of_measurement=UNIT_SECOND,
                    icon=ICON_TIMELAPSE,
                    accuracy_decimals=0,
                ),
                key=CONF_NAME,
            ),
            cv.Optional(CONF_ILLUMINANCE): cv.maybe_simple_value(
                sensor.sensor_schema(
                    unit_of_measurement=UNIT_LUX,
                    accuracy_decimals=0,
                    device_class=DEVICE_CLASS_ILLUMINANCE,
                    state_class=STATE_CLASS_MEASUREMENT,
                ),
                key=CONF_NAME,
            ),
            cv.Optional(CONF_TIMEOUT, "0s"): cv.positive_time_period_milliseconds,
        },
    )
    .extend(miot.MIOT_BLE_DEVICE_SCHEMA)
)


async def to_code(config):
    """Code generation entry point"""
    var = await miot.new_binary_sensor_device(config)

    if CONF_LIGHT in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_LIGHT])
        cg.add(var.set_light_binary_sensor(sens))
    if CONF_IDLE_TIME in config:
        sens = await sensor.new_sensor(config[CONF_IDLE_TIME])
        cg.add(var.set_idle_time_sensor(sens))
    if CONF_ILLUMINANCE in config:
        sens = await sensor.new_sensor(config[CONF_ILLUMINANCE])
        cg.add(var.set_illuminance_sensor(sens))

    cg.add(var.set_reset_timeout(config[CONF_TIMEOUT].total_milliseconds))
