import logging
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import CORE
from esphome.components import ble_client, miot
from esphome.const import (
    CONF_ID,
    CONF_BINDKEY,
    CONF_USERNAME,
    CONF_PASSWORD,
    CONF_SERVERS,
    CONF_UPDATE_INTERVAL,
    CONF_NAME,
    CONF_MAC_ADDRESS,
)

# from esphome.const import ESP_PLATFORM_ESP32
_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@dentra"]
# ESP_PLATFORMS = [ESP_PLATFORM_ESP32]
DEPENDENCIES = ["ble_client"]

CONF_TOKEN = "token"
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


def cv_token(value):
    value = cv.string_strict(value)
    parts = [value[i : i + 2] for i in range(0, len(value), 2)]
    if len(parts) != 12:
        raise cv.Invalid("Token must consist of 12 hexadecimal numbers")
    parts_int = []
    if any(len(part) != 2 for part in parts):
        raise cv.Invalid("Token must be format XX")
    for part in parts:
        try:
            parts_int.append(int(part, 16))
        except ValueError:
            # pylint: disable=raise-missing-from
            raise cv.Invalid("Token must be hex values from 00 to FF")

    return "".join(f"{part:02X}" for part in parts_int)


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
    return _auth_schema(MiotStandardAuthClient).extend(
        {
            cv.Optional(CONF_BINDKEY): cv.bind_key,
            cv.Optional(CONF_TOKEN): cv_token,
        }
    )


def secure_auth_schema() -> cv.Schema:
    return _auth_schema(MiotSecureAuthClient)


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


async def register_standard_auth_client(config):
    auth = cg.new_Pvariable(config[CONF_AUTH_CLIENT_ID])
    await ble_client.register_ble_node(auth, config)
    if CONF_AUTH_CLIENT_DEBUG in config:
        cg.add(auth.set_debug(True))

    if CONF_BINDKEY in config:
        cg.add(auth.set_bindkey(miot.as_bindkey(config[CONF_BINDKEY])))
    if CONF_TOKEN in config:
        cg.add(auth.set_token(as_token(config[CONF_TOKEN])))

    if CONF_BINDKEY not in config or CONF_TOKEN not in config:
        conf = CORE.config["miot"].get(miot.CONF_XIAOMI_ACCOUNT)
        if conf:
            xbk = miot.XiaomiBeaconkeys(
                username=conf[CONF_USERNAME],
                password=conf[CONF_PASSWORD],
                servers=conf[CONF_SERVERS],
                storage_path=CORE.build_path,
                update_interval=conf[CONF_UPDATE_INTERVAL].total_seconds,
            )
            blec_id = auth[ble_client.CONF_BLE_CLIENT_ID]
            mac = None
            for blec in CORE.config["ble_client"]:
                if blec[CONF_ID].id == blec_id:
                    mac = blec[CONF_MAC_ADDRESS]
                    break
            if mac is None:
                _LOGGER.error("No mac address found for %s", blec_id)
            if CONF_BINDKEY not in config:
                bindkey = xbk.get_beaconkey(mac)
                if bindkey:
                    _LOGGER.info(
                        "Got bindkey for %s %s", mac, config.get(CONF_NAME, "")
                    )
                    cg.add(auth.set_bindkey(miot.as_bindkey(bindkey)))
                else:
                    _LOGGER.error("No bindkey found for %s", blec_id)
            if CONF_TOKEN not in config:
                token = xbk.get_token(mac)
                if token:
                    _LOGGER.info("Got token for %s %s", mac, config.get(CONF_NAME, ""))
                    cg.add(auth.set_token(as_token(token)))
                else:
                    _LOGGER.error("No token found for %s", blec_id)

    return auth
