import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import DEVICE_CLASS_MOISTURE
from .. import miot  # pylint: disable=relative-beyond-top-level

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

CONF_BOTTOM = "bottom"
CONF_TOP = "top"

miot_sjws02lm_ns = cg.esphome_ns.namespace("miot_sjws02lm")
MiotSJWS02LM = miot_sjws02lm_ns.class_(
    "MiotSJWS02LM", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(
        MiotSJWS02LM, device_class=DEVICE_CLASS_MOISTURE
    )
    .extend(
        {
            cv.Optional(CONF_BOTTOM): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_MOISTURE
            ),
            cv.Optional(CONF_TOP): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_MOISTURE
            ),
        }
    )
    .extend(miot.MIOT_BLE_DEVICE_SCHEMA)
)


async def to_code(config):
    """Code generation entry point"""
    var = await miot.new_binary_sensor_device(config)
    if CONF_BOTTOM in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_BOTTOM])
        cg.add(var.set_bottom(sens))
    if CONF_TOP in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_TOP])
        cg.add(var.set_top(sens))
