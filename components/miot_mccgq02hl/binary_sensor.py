import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    CONF_LIGHT,
    DEVICE_CLASS_LIGHT,
    DEVICE_CLASS_OPENING,
    DEVICE_CLASS_PROBLEM,
)
from .. import miot  # pylint: disable=relative-beyond-top-level

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

CONF_ALERT = "alert"

miot_mccgq02hl = cg.esphome_ns.namespace("miot_mccgq02hl")
MiotMCCGQ02HL = miot_mccgq02hl.class_(
    "MiotMCCGQ02HL", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(MiotMCCGQ02HL, device_class=DEVICE_CLASS_OPENING)
    .extend(
        {
            cv.Optional(CONF_LIGHT): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_LIGHT
            ),
            cv.Optional(CONF_ALERT): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_PROBLEM
            ),
        }
    )
    .extend(miot.MIOT_BLE_DEVICE_SCHEMA)
)


async def to_code(config):
    """Code generation entry point"""
    var = await miot.new_binary_sensor_device(config)
    if CONF_LIGHT in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_LIGHT])
        cg.add(var.set_light(sens))
    if CONF_ALERT in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_ALERT])
        cg.add(var.set_alert(sens))
