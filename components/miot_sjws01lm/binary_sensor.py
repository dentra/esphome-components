import esphome.codegen as cg
from esphome.components import binary_sensor
from esphome.const import DEVICE_CLASS_MOISTURE
from .. import miot  # pylint: disable=relative-beyond-top-level

DEVICE_CLASS_FLOODING = DEVICE_CLASS_MOISTURE

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

miot_sjws01lm_ns = cg.esphome_ns.namespace("miot_sjws01lm")
MiotSJWS01LM = miot_sjws01lm_ns.class_(
    "MiotSJWS01LM", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(
    MiotSJWS01LM, device_class=DEVICE_CLASS_FLOODING
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def to_code(config):
    """Code generation entry point"""
    await miot.new_binary_sensor_device(config)
