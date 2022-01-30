import logging
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import CORE
from esphome.components import ble_client
from esphome.const import (
    CONF_ID,
    CONF_MAC_ADDRESS,
    PLATFORM_ESP32,
)
from .. import xiaomi_account

# from esphome.const import ESP_PLATFORM_ESP32
_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@dentra"]
ESP_PLATFORMS = [PLATFORM_ESP32]
DEPENDENCIES = ["ble_client"]

CONF_AUTH_CLIENT_ID = "auth_client_id"
CONF_AUTH_CLIENT_DEBUG = "auth_client_debug"
CONF_MIOT_CLIENT_ID = "miot_client_id"
CONF_MIOT_CLIENT_DEBUG = "miot_client_debug"

miot_client_ns = cg.esphome_ns.namespace("miot_client")
MiotClient = miot_client_ns.class_("MiotClient", ble_client.BLEClientNode)
AuthClient = miot_client_ns.class_("AuthClient")
AuthClientListener = miot_client_ns.class_("AuthClientListener")
MiotLegacyBondClient = miot_client_ns.class_(
    "MiotLegacyBondClient", MiotClient, AuthClient
)
MiotStandardAuthClient = miot_client_ns.class_(
    "MiotStandardAuthClient", MiotClient, AuthClient
)
MiotSecureAuthClient = miot_client_ns.class_(
    "MiotSecureAuthClient", MiotClient, AuthClient
)


def as_token(value):
    cpp_array = [
        f"0x{part}" for part in [value[i : i + 2] for i in range(0, len(value), 2)]
    ]
    return cg.RawExpression("(const uint8_t[12]){{{}}}".format(",".join(cpp_array)))


CONFIG_SCHEMA = cv.Schema({})


def client_schema(node) -> cv.Schema:
    return cv.Schema(
        {
            cv.GenerateID(CONF_MIOT_CLIENT_ID): cv.declare_id(node),
            cv.Optional(CONF_MIOT_CLIENT_DEBUG): cv.boolean,
        }
    )


def _auth_schema(auth) -> cv.Schema:
    return cv.Schema(
        {
            cv.GenerateID(CONF_AUTH_CLIENT_ID): cv.declare_id(auth),
            cv.Optional(CONF_AUTH_CLIENT_DEBUG): cv.boolean,
        }
    )


def legacy_auth_schema() -> cv.Schema:
    return _auth_schema(MiotLegacyBondClient)


def standard_auth_schema() -> cv.Schema:
    return (
        _auth_schema(MiotStandardAuthClient)
        .extend(xiaomi_account.BINDKEY_SCHEMA)
        .extend(xiaomi_account.TOKEN_SCHEMA)
    )


def secure_auth_schema() -> cv.Schema:
    return _auth_schema(MiotSecureAuthClient)


async def register_client(config, parent=None):
    node = cg.new_Pvariable(config[CONF_MIOT_CLIENT_ID], parent)
    await ble_client.register_ble_node(node, config)
    if config.get(CONF_MIOT_CLIENT_DEBUG, False):
        cg.add(node.set_debug(True))
    return node


async def register_legacy_auth_client(config, product_id=None):
    auth = cg.new_Pvariable(config[CONF_AUTH_CLIENT_ID])
    await ble_client.register_ble_node(auth, config)
    if config.get(CONF_AUTH_CLIENT_DEBUG, False):
        cg.add(auth.set_debug(True))
    if product_id:
        cg.add(auth.set_product_id(product_id))
    return auth


async def register_standard_auth_client(config):
    auth = cg.new_Pvariable(config[CONF_AUTH_CLIENT_ID])
    await ble_client.register_ble_node(auth, config)
    if config.get(CONF_AUTH_CLIENT_DEBUG, False):
        cg.add(auth.set_debug(True))

    blec_id = auth[ble_client.CONF_BLE_CLIENT_ID]
    mac = None
    for blec in CORE.config["ble_client"]:
        if blec[CONF_ID].id == blec_id:
            mac = blec[CONF_MAC_ADDRESS]
            break
    if mac is None:
        _LOGGER.error("No mac address found for %s", blec_id)

    xiaomi_account.set_bindkey(config, mac, auth.set_bindkey)
    xiaomi_account.set_token(config, mac, auth.set_token)

    return auth
