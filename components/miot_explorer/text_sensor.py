from esphome.const import CONF_NAME
from esphome.components.esp32_ble_tracker import CONF_ESP32_BLE_ID
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import miot, text_sensor, sensor
from esphome.const import (
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
)

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot", "sensor", "text_sensor", "switch"]

CONF_CONSUMABLE = "consumable"

miot_explorer_ns = cg.esphome_ns.namespace("miot_explorer")
MiotExplorer = miot_explorer_ns.class_(
    "MiotExplorer", miot.MiotComponent, text_sensor.TextSensor
)

CONFIG_SCHEMA = text_sensor.TEXT_SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MiotExplorer),
        cv.Required(CONF_NAME): cv.string,
        cv.Optional(CONF_CONSUMABLE): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def to_code(config):
    var = await miot.new_text_sensor_device(config)
    if CONF_CONSUMABLE in config:
        conf = config[CONF_CONSUMABLE]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_consumable(sens))
