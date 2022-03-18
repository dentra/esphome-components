import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor
from esphome.const import (
    CONF_IDLE_TIME,
    CONF_LIGHT,
    CONF_TIMEOUT,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_LIGHT,
    DEVICE_CLASS_MOTION,
    ICON_TIMELAPSE,
    UNIT_SECOND,
)
from .. import miot  # pylint: disable=relative-beyond-top-level

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

miot_rtcgq2lm_ns = cg.esphome_ns.namespace("miot_rtcgq02lm")
MiotRTCGQ02LM = miot_rtcgq2lm_ns.class_(
    "MiotRTCGQ02LM", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(MiotRTCGQ02LM, device_class=DEVICE_CLASS_MOTION)
    .extend(
        {
            cv.Optional(CONF_LIGHT): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_LIGHT
            ),
            cv.Optional(CONF_TIMEOUT): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_IDLE_TIME): sensor.sensor_schema(
                unit_of_measurement=UNIT_SECOND,
                icon=ICON_TIMELAPSE,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_EMPTY,
                # entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            # cv.Optional(CONF_ILLUMINANCE): sensor.sensor_schema(
            #     UNIT_LUX, ICON_EMPTY, 0, DEVICE_CLASS_ILLUMINANCE, STATE_CLASS_MEASUREMENT
            # ),
        },
    )
    .extend(miot.MIOT_BLE_DEVICE_SCHEMA)
)


async def to_code(config):
    """Code generation entry point"""
    var = await miot.new_binary_sensor_device(config)
    if CONF_LIGHT in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_LIGHT])
        cg.add(var.set_light(sens))
    if CONF_TIMEOUT in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_TIMEOUT])
        cg.add(var.set_timeout(sens))
    if CONF_IDLE_TIME in config:
        sens = await sensor.new_sensor(config[CONF_IDLE_TIME])
        cg.add(var.set_idle_time(sens))
    # if CONF_ILLUMINANCE in config:
    #     sens = await sensor.new_sensor(config[CONF_ILLUMINANCE])
    #     cg.add(var.set_illuminance(sens))
