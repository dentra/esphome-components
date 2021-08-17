import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import miot, binary_sensor
from esphome.const import (
    CONF_DEVICE_CLASS,
    CONF_LIGHT,
    DEVICE_CLASS_LIGHT,
    DEVICE_CLASS_OPENING,
)

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

CONF_ALERT = "alert"

miot_mccgq02hl = cg.esphome_ns.namespace("miot_mccgq02hl")
MiotMCCGQ02HL = miot_mccgq02hl.class_(
    "MiotMCCGQ02HL", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = binary_sensor.BINARY_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MiotMCCGQ02HL),
        cv.Optional(
            CONF_DEVICE_CLASS, default=DEVICE_CLASS_OPENING
        ): binary_sensor.device_class,
        cv.Optional(CONF_LIGHT): binary_sensor.BINARY_SENSOR_SCHEMA.extend(
            {
                cv.Optional(
                    CONF_DEVICE_CLASS, default=DEVICE_CLASS_LIGHT
                ): binary_sensor.device_class,
            }
        ),
        cv.Optional(CONF_ALERT): binary_sensor.BINARY_SENSOR_SCHEMA.extend(
            {
                # cv.Optional(
                #     CONF_DEVICE_CLASS, default=DEVICE_CLASS_EMPTY
                # ): binary_sensor.device_class,
            }
        ),
    }
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def to_code(config):
    var = await miot.new_binary_sensor_device(config)
    if CONF_LIGHT in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_LIGHT])
        cg.add(var.set_light(sens))
    if CONF_ALERT in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_ALERT])
        cg.add(var.set_alert(sens))
