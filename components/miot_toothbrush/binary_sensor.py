import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor
from esphome.const import (
    UNIT_PERCENT,
    UNIT_SECOND,
    STATE_CLASS_MEASUREMENT,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_OCCUPANCY,
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from .. import miot  # pylint: disable=relative-beyond-top-level

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

CONF_SCORE = "score"
CONF_CONSUMABLE = "consumable_level"
CONF_BRUSHING_TIME = "brushing_time"

miot_toothbrush_ns = cg.esphome_ns.namespace("miot_toothbrush")
MiotToothbrush = miot_toothbrush_ns.class_(
    "MiotToothbrush", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(
        MiotToothbrush,
        device_class=DEVICE_CLASS_OCCUPANCY,
        icon="mdi:scoreboard-outline",
    )
    .extend(
        {
            cv.Optional(CONF_SCORE): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                icon="mdi:scoreboard-outline",
            ),
            cv.Optional(CONF_CONSUMABLE): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:toothbrush",
            ),
            cv.Optional(CONF_BRUSHING_TIME): sensor.sensor_schema(
                unit_of_measurement=UNIT_SECOND,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_DURATION,
                icon="mdi:timer-sand-complete",
            ),
        },
    )
    .extend(miot.MIOT_BLE_DEVICE_SCHEMA)
)


async def to_code(config):
    """Code generation entry point"""
    var = await miot.new_binary_sensor_device(config)
    if CONF_SCORE in config:
        sens = await sensor.new_sensor(config[CONF_SCORE])
        cg.add(var.set_score(sens))
    if CONF_CONSUMABLE in config:
        sens = await sensor.new_sensor(config[CONF_CONSUMABLE])
        cg.add(var.set_consumable_level(sens))
    if CONF_BRUSHING_TIME in config:
        sens = await sensor.new_sensor(config[CONF_BRUSHING_TIME])
        cg.add(var.set_brushing_time(sens))
