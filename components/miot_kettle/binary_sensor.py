import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, sensor
from esphome.const import (
    CONF_TEMPERATURE,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    UNIT_CELSIUS,
    STATE_CLASS_MEASUREMENT,
)
from .. import miot  # pylint: disable=relative-beyond-top-level


CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot", "sensor"]

miot_kettle_ns = cg.esphome_ns.namespace("miot_kettle")
MiotKettle = miot_kettle_ns.class_(
    "MiotKettle", miot.MiotComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(MiotKettle, device_class=DEVICE_CLASS_POWER)
    .extend(
        {
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        },
    )
    .extend(miot.MIOT_BLE_DEVICE_SCHEMA)
)


async def to_code(config):
    """Code generation entry point"""
    var = await miot.new_binary_sensor_device(config)
    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature(sens))
