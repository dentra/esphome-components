import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client, miot, miot_client, text_sensor
from esphome.const import (
    CONF_ID,
    CONF_VERSION,
)
from esphome.cpp_types import Component

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot", "miot_client", "text_sensor", "ble_client"]

ICON_VERSION = "mdi:cellphone-arrow-down"

miot_ylxx0xyl_ns = cg.esphome_ns.namespace("miot_ylxx0xyl")
MiotYLxx0xYLPair = miot_ylxx0xyl_ns.class_(
    "MiotYLxx0xYLPair", Component, miot.MiotListener, text_sensor.TextSensor
)
MiotYLxx0xYLNode = miot_ylxx0xyl_ns.class_(
    "MiotYLxx0xYLNode", miot_client.MiotClient, miot_client.AuthClientListener
)

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(MiotYLxx0xYLPair)
    .extend(
        {
            cv.GenerateID(miot.CONF_MIOT_ID): cv.use_id(miot.MiBeaconTracker),
            cv.Optional(CONF_VERSION): text_sensor.text_sensor_schema(
                text_sensor.TextSensor, icon=ICON_VERSION
            ),
            cv.Optional(miot.CONF_PRODUCT_ID): cv.uint16_t,
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
    .extend(miot_client.client_schema(MiotYLxx0xYLNode))
    .extend(miot_client.legacy_auth_schema())
)


async def to_code(config):
    # var = await miot.new_text_sensor_device(config)
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await miot.register_miot_device(var, config)
    await text_sensor.register_text_sensor(var, config)

    auth = await miot_client.register_legacy_auth_client(config)
    node = await miot_client.register_client(config, var)
    cg.add(auth.add_auth_listener(node))
    cg.add(node.set_auth_client(auth))
    cg.add(var.set_auth_client(auth))

    blec = await cg.get_variable(config[ble_client.CONF_BLE_CLIENT_ID])
    cg.add(var.set_address(blec.get_address()))

    if CONF_VERSION in config:
        sens = cg.new_Pvariable(config[CONF_VERSION][CONF_ID])
        await text_sensor.register_text_sensor(sens, config[CONF_VERSION])
        cg.add(var.set_version(sens))

    if miot.CONF_PRODUCT_ID in config:
        cg.add(var.set_product_id(config[miot.CONF_PRODUCT_ID]))
