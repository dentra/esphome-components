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
    ICON_TIMELAPSE,
    STATE_CLASS_MEASUREMENT,
    UNIT_LUX,
    UNIT_SECOND,
)

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

CONF_DETECT_NO_MOTION_ASAP = "detect_no_motion_asap"

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
        cv.Optional(CONF_DETECT_NO_MOTION_ASAP): cv.boolean,
        cv.Optional(CONF_LIGHT): binary_sensor.BINARY_SENSOR_SCHEMA.extend(
            {
                cv.Optional(
                    CONF_DEVICE_CLASS, default=DEVICE_CLASS_LIGHT
                ): binary_sensor.device_class,
            }
        ),
        cv.Optional(CONF_IDLE_TIME): sensor.sensor_schema(
            unit_of_measurement=UNIT_SECOND,
            icon=ICON_TIMELAPSE,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            # entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_ILLUMINANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_LUX,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_ILLUMINANCE,
            state_class=STATE_CLASS_MEASUREMENT,
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
    if CONF_DETECT_NO_MOTION_ASAP in config:
        if config[CONF_DETECT_NO_MOTION_ASAP]:
            cg.add(var.set_detect_no_motion_asap(True))
