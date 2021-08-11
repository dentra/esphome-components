import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor, esp32_ble_tracker
from esphome.const import (
    CONF_DEVICE_CLASS,
    CONF_IDLE_TIME,
    CONF_ILLUMINANCE,
    CONF_LIGHT,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_ILLUMINANCE,
    DEVICE_CLASS_LIGHT,
    DEVICE_CLASS_MOTION,
    ICON_EMPTY,
    ICON_TIMELAPSE,
    STATE_CLASS_MEASUREMENT,
    UNIT_LUX,
    UNIT_MINUTE,
    UNIT_SECOND,
)

from esphome.components import miot

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

miot_rtcgq2lm_ns = cg.esphome_ns.namespace("miot_rtcgq2lm")
MiotRTCGQ2LM = miot_rtcgq2lm_ns.class_(
    "MiotRTCGQ2LM", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = binary_sensor.BINARY_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MiotRTCGQ2LM),
        cv.Optional(
            CONF_DEVICE_CLASS, default=DEVICE_CLASS_MOTION
        ): binary_sensor.device_class,
        cv.Optional(CONF_LIGHT): binary_sensor.BINARY_SENSOR_SCHEMA.extend(
            {
                cv.Optional(
                    CONF_DEVICE_CLASS, default=DEVICE_CLASS_LIGHT
                ): binary_sensor.device_class,
            }
        ),
        cv.Optional(CONF_IDLE_TIME): sensor.sensor_schema(
            UNIT_SECOND, ICON_TIMELAPSE, 0, DEVICE_CLASS_EMPTY
        ),
        cv.Optional(CONF_ILLUMINANCE): sensor.sensor_schema(
            UNIT_LUX, ICON_EMPTY, 0, DEVICE_CLASS_ILLUMINANCE, STATE_CLASS_MEASUREMENT
        ),
        cv.Optional(CONF_): sensor.sensor_schema(
            UNIT_SECOND, ICON_TIMELAPSE, 0, DEVICE_CLASS_EMPTY
        ),
    },
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def to_code(config):
    var = await miot.new_binary_sensor_device(config)
    if CONF_LIGHT in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_LIGHT])
        cg.add(var.set_light(sens))
    if CONF_IDLE_TIME in config:
        sens = await sensor.new_sensor(config[CONF_IDLE_TIME])
        cg.add(var.set_idle_time(sens))
    if CONF_ILLUMINANCE in config:
        sens = await sensor.new_sensor(config[CONF_ILLUMINANCE])
        cg.add(var.set_illuminance(sens))
