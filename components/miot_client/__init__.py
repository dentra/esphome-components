import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client
# from esphome.const import ESP_PLATFORM_ESP32

CODEOWNERS = ["@dentra"]
# ESP_PLATFORMS = [ESP_PLATFORM_ESP32]
DEPENDENCIES = ["ble_client"]

CONF_AUTH_CLIENT_ID = "auth_client_id"
CONF_AUTH_CLIENT_DEBUG = "auth_client_debug"
CONF_MIOT_CLIENT_ID = "miot_client_id"
CONF_MIOT_CLIENT_DEBUG = "miot_client_debug"

miot_client_ns = cg.esphome_ns.namespace("miot_client")
MiotClient = miot_client_ns.class_("MiotClient", ble_client.BLEClientNode)
AuthClient = miot_client_ns.class_("AuthClient")
AuthClientListener = miot_client_ns.class_("AuthClientListener")
MiotLegacyAuthClient = miot_client_ns.class_(
    "MiotLegacyAuthClient", MiotClient, AuthClient
)
MiotStandardAuthClient = miot_client_ns.class_(
    "MiotStandardAuthClient", MiotClient, AuthClient
)
MiotSecureAuthClient = miot_client_ns.class_(
    "MiotSecureAuthClient", MiotClient, AuthClient
)

CONFIG_SCHEMA = cv.Schema({})


def client_schema(node) -> cv.Schema:
    return cv.Schema(
        {
            cv.GenerateID(CONF_MIOT_CLIENT_ID): cv.declare_id(node),
            cv.Optional(CONF_MIOT_CLIENT_DEBUG): cv.boolean,
        }
    )


def _auth_schema(auth, node) -> cv.Schema:
    return client_schema(node).extend(
        {
            cv.GenerateID(CONF_AUTH_CLIENT_ID): cv.declare_id(auth),
            cv.Optional(CONF_AUTH_CLIENT_DEBUG): cv.boolean,
        }
    )


def legacy_auth_schema(node) -> cv.Schema:
    return _auth_schema(MiotLegacyAuthClient, node)


def standard_auth_schema(node) -> cv.Schema:
    return _auth_schema(MiotStandardAuthClient, node)


def secure_auth_schema(node) -> cv.Schema:
    return _auth_schema(MiotSecureAuthClient, node)


async def register_client(config, parent=None):
    node = cg.new_Pvariable(config[CONF_MIOT_CLIENT_ID], parent)
    await ble_client.register_ble_node(node, config)
    if CONF_MIOT_CLIENT_DEBUG in config:
        cg.add(node.set_debug(True))
    return node


async def register_legacy_auth_client(config, product_id=None):
    auth = cg.new_Pvariable(config[CONF_AUTH_CLIENT_ID])
    await ble_client.register_ble_node(auth, config)
    if CONF_AUTH_CLIENT_DEBUG in config:
        cg.add(auth.set_debug(True))
    if product_id:
        cg.add(auth.set_product_id(product_id))
    return auth
