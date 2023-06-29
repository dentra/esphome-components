import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import UNIT_PERCENT, STATE_CLASS_MEASUREMENT
from .. import miot  # pylint: disable=relative-beyond-top-level

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

CONF_CONSUMABLE = "consumable_level"

miot_toothbrush_ns = cg.esphome_ns.namespace("miot_toothbrush")
MiotToothbrush = miot_toothbrush_ns.class_(
    "MiotToothbrush", miot.MiotComponent, sensor.Sensor
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(MiotToothbrush, accuracy_decimals=0)
    .extend(
        {
            cv.Optional(CONF_CONSUMABLE): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        },
    )
    .extend(miot.MIOT_BLE_DEVICE_SCHEMA)
)


async def to_code(config):
    """Code generation entry point"""
    var = await miot.new_sensor_device(config)
    if CONF_CONSUMABLE in config:
        sens = await sensor.new_sensor(config[CONF_CONSUMABLE])
        cg.add(var.set_consumable_level(sens))
