from esphome.const import CONF_NAME
from esphome.components.esp32_ble_tracker import CONF_ESP32_BLE_ID
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import miot, text_sensor, ble_client

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot", "sensor", "text_sensor", "switch"]

miot_explorer_ns = cg.esphome_ns.namespace("miot_explorer")
MiotExplorer = miot_explorer_ns.class_(
    "MiotExplorer", miot.MiotComponent, text_sensor.TextSensor
)

CONFIG_SCHEMA = text_sensor.TEXT_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MiotExplorer),
        cv.Required(CONF_NAME): cv.string,
    }
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def to_code(config):
    await miot.new_text_sensor_device(config)
