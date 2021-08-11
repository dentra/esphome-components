import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor, esp32_ble_tracker
from esphome.const import (
    CONF_DEVICE_CLASS,
    DEVICE_CLASS_MOISTURE,
)

from esphome.components import miot

DEVICE_CLASS_FLOODING = DEVICE_CLASS_MOISTURE

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

miot_sjws01lm_ns = cg.esphome_ns.namespace("miot_sjws01lm")
MiotSJWS01LM = miot_sjws01lm_ns.class_(
    "MiotSJWS01LM", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = binary_sensor.BINARY_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MiotSJWS01LM),
        cv.Optional(
            CONF_DEVICE_CLASS, default=DEVICE_CLASS_FLOODING
        ): binary_sensor.device_class,
    },
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def to_code(config):
    await miot.new_binary_sensor_device(config)
