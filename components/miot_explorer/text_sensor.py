import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import miot, text_sensor

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

miot_explorer_ns = cg.esphome_ns.namespace("miot_explorer")
MiotExplorer = miot_explorer_ns.class_(
    "MiotExplorer", miot.MiotComponent, text_sensor.TextSensor
)

CONFIG_SCHEMA = text_sensor.TEXT_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MiotExplorer),
    }
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def to_code(config):
    var = await miot.new_device(config)
    await text_sensor.register_text_sensor(var, config)
