import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import miot, sensor, binary_sensor
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
    UNIT_SECOND,
)

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

miot_cgpr1_ns = cg.esphome_ns.namespace("miot_cgpr1")
MiotCGPR1 = miot_cgpr1_ns.class_(
    "MiotCGPR1", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = binary_sensor.BINARY_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MiotCGPR1),
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
    },
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def to_code(config):
    var = await miot.new_binary_sensor_device(config)
    if CONF_IDLE_TIME in config:
        sens = await sensor.new_sensor(config[CONF_IDLE_TIME])
        cg.add(var.set_idle_time(sens))
    if CONF_ILLUMINANCE in config:
        sens = await sensor.new_sensor(config[CONF_ILLUMINANCE])
        cg.add(var.set_illuminance(sens))
