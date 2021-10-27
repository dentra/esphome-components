import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import miot, binary_sensor, sensor
from esphome.const import (
    CONF_DEVICE_CLASS,
    CONF_TEMPERATURE,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    UNIT_CELSIUS,
    STATE_CLASS_MEASUREMENT,
    ICON_EMPTY,
)


CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot", "sensor"]

miot_kettle_ns = cg.esphome_ns.namespace("miot_kettle")
MiotKettle = miot_kettle_ns.class_(
    "MiotKettle", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = binary_sensor.BINARY_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MiotKettle),
        cv.Optional(
            CONF_DEVICE_CLASS, default=DEVICE_CLASS_POWER
        ): binary_sensor.device_class,
        cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
            UNIT_CELSIUS,
            ICON_EMPTY,
            1,
            DEVICE_CLASS_TEMPERATURE,
            STATE_CLASS_MEASUREMENT,
        ),
    },
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def to_code(config):
    var = await miot.new_binary_sensor_device(config)
    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature(sens))
