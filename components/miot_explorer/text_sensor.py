import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import miot, sensor, text_sensor
from esphome.core import CORE
from esphome.const import (
    CONF_NAME,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
)

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot", "sensor", "text_sensor", "switch"]

CONF_CONSUMABLE = "consumable"
CONF_MAX_SENSORS = "max_sensors"

miot_explorer_ns = cg.esphome_ns.namespace("miot_explorer")
MiotExplorer = miot_explorer_ns.class_(
    "MiotExplorer", miot.MiotComponent, text_sensor.TextSensor
)

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(MiotExplorer)
    .extend(
        {
            cv.Required(CONF_NAME): cv.string,
            cv.Optional(CONF_MAX_SENSORS, default=16): cv.int_range(min=1, max=255),
            cv.Optional(CONF_CONSUMABLE): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
        }
    )
    .extend(miot.MIOT_BLE_DEVICE_SCHEMA)
)


async def to_code(config):
    var = await miot.new_text_sensor_device(config)
    cg.add(var.set_max_sensors(config[CONF_MAX_SENSORS]))
    # reserve entity slots for dynamically created sensors
    for _ in range(config[CONF_MAX_SENSORS]):
        CORE.register_platform_component("text_sensor", None)
    if CONF_CONSUMABLE in config:
        conf = config[CONF_CONSUMABLE]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_consumable(sens))
