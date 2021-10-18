import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, miot
from esphome.const import (
    CONF_HUMIDITY,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_TEMPERATURE,
    ICON_EMPTY,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_PERCENT,
)

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

miot_mhoc303_ns = cg.esphome_ns.namespace("miot_mhoc303")
MiotmMHOC303 = miot_mhoc303_ns.class_("MiotmMHOC303", miot.MiotComponent, sensor.Sensor)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        UNIT_CELSIUS,
        ICON_EMPTY,
        1,
        DEVICE_CLASS_TEMPERATURE,
        STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(MiotmMHOC303),
            cv.Optional(CONF_HUMIDITY): sensor.sensor_schema(
                UNIT_PERCENT,
                ICON_EMPTY,
                1,
                DEVICE_CLASS_HUMIDITY,
                STATE_CLASS_MEASUREMENT,
            ),
        },
    )
    .extend(miot.MIOT_BLE_DEVICE_SCHEMA)
)


async def to_code(config):
    var = await miot.new_sensor_device(config)
    if CONF_HUMIDITY in config:
        sens = await sensor.new_sensor(config[CONF_HUMIDITY])
        cg.add(var.set_humidity(sens))
