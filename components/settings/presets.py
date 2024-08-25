from collections import OrderedDict
from typing import Any

import esphome.codegen as cg
from esphome.components import api, esp32, mqtt, uart, web_server, wifi
from esphome.const import (
    CONF_AREA,
    CONF_COMMENT,
    CONF_ESPHOME,
    CONF_FRIENDLY_NAME,
    CONF_ID,
    CONF_MQTT,
    CONF_NAME,
    CONF_WEB_SERVER,
    CONF_WIFI,
)
from esphome.core import CORE
from esphome.core.config import CONF_NAME_ADD_MAC_SUFFIX

from .. import cgp
from . import const, cpp, var

PRESET_API = "api"
PRESET_BLE = "ble"
PRESET_UART = "uart"
PRESET_NODE = "node"
PRESET_WIFI = CONF_WIFI
PRESET_MQTT = CONF_MQTT
PRESET_WEB_SERVER = CONF_WEB_SERVER

COMP_API = "api"
COMP_UART = "uart"
COMP_WIFI = CONF_WIFI
COMP_MQTT = CONF_MQTT
COMP_ESPHOME = CONF_ESPHOME
COMP_BLE_CLIENT = "ble_client"
COMP_WEB_SERVER = CONF_WEB_SERVER

KEY_NODE_NAME = "node_name"
KEY_NODE_NAME_ADD_MAC_SUFFIX = "node_name_add_mac_suffix"
KEY_WIFI_SSID = "wifi_network0_ssid"
KEY_WIFI_PASSWORD = "wifi_network0_password"

USE_ARDUINO = "USE_ARDUINO"
USE_API = "USE_API"
USE_ESP32 = "USE_ESP32"
USE_MQTT = "USE_MQTT"
USE_WEB_SERVER = "USE_WEBSERVER"
USE_WIFI = "USE_WIFI"

_PRESET_DEFAULTS = {
    PRESET_UART: {
        const.CONF_VAR_PRESET: PRESET_UART,
        const.CONF_VAR_SECTION: "UART",
        const.CONF_VAR_COMP: COMP_UART,
    },
    PRESET_BLE: {
        const.CONF_VAR_PRESET: PRESET_BLE,
        const.CONF_VAR_SECTION: "BLE",
        const.CONF_VAR_COMP: COMP_BLE_CLIENT,
        const.CONF_VAR_IFDEF: USE_ESP32,
    },
    PRESET_NODE: {
        const.CONF_VAR_PRESET: PRESET_NODE,
        const.CONF_VAR_SECTION: "Node",
        const.CONF_VAR_COMP: COMP_ESPHOME,
    },
    PRESET_WIFI: {
        const.CONF_VAR_PRESET: PRESET_WIFI,
        const.CONF_VAR_SECTION: "WiFi",
        const.CONF_VAR_COMP: COMP_WIFI,
        const.CONF_VAR_IFDEF: USE_WIFI,
    },
    PRESET_API: {
        const.CONF_VAR_PRESET: PRESET_API,
        const.CONF_VAR_SECTION: "API",
        const.CONF_VAR_COMP: COMP_API,
        const.CONF_VAR_IFDEF: USE_API,
    },
    PRESET_MQTT: {
        const.CONF_VAR_PRESET: PRESET_MQTT,
        const.CONF_VAR_SECTION: "MQTT",
        const.CONF_VAR_COMP: COMP_MQTT,
        const.CONF_VAR_IFDEF: USE_MQTT,
    },
    PRESET_WEB_SERVER: {
        const.CONF_VAR_PRESET: PRESET_WEB_SERVER,
        const.CONF_VAR_SECTION: "Web Server",
        const.CONF_VAR_COMP: COMP_WEB_SERVER,
        const.CONF_VAR_IFDEF: USE_WEB_SERVER,
    },
}

_PRESETS = OrderedDict(
    {
        PRESET_UART: OrderedDict(
            {
                "uart0_tx_pin": {
                    const.CONF_VAR_TYPE: var.VT_PIN,
                    const.CONF_VAR_NAME: "TX Pin",
                    const.CONF_VAR_COMP_IDX: 0,
                    const.CONF_VAR_COMP_KEY: uart.CONF_TX_PIN,
                    const.CONF_VAR_GETTER: lambda _, o: o.get_pin(),
                    const.CONF_VAR_SETTER: lambda _, o: o.set_pin,
                },
                "uart0_rx_pin": {
                    const.CONF_VAR_TYPE: var.VT_PIN,
                    const.CONF_VAR_NAME: "RX Pin",
                    const.CONF_VAR_COMP_IDX: 0,
                    const.CONF_VAR_COMP_KEY: uart.CONF_RX_PIN,
                    const.CONF_VAR_GETTER: lambda _, o: o.get_pin(),
                    const.CONF_VAR_SETTER: lambda _, o: o.set_pin,
                },
            }
        ),
        PRESET_BLE: OrderedDict(
            {
                "ble_client0_mac_address": {
                    const.CONF_VAR_TYPE: var.VT_MAC,
                    const.CONF_VAR_NAME: "MAC-address",
                    const.CONF_VAR_COMP_IDX: 0,
                    const.CONF_VAR_GETTER: lambda _, o: o.get_address(),
                    const.CONF_VAR_SETTER: lambda _, o: o.set_address,
                }
            }
        ),
        PRESET_NODE: OrderedDict(
            {
                KEY_NODE_NAME: {
                    const.CONF_VAR_TYPE: var.VT_STR,
                    const.CONF_VAR_MIN: 1,
                    const.CONF_VAR_MAX: 24,
                    const.CONF_VAR_NAME: "Name",
                    const.CONF_VAR_HELP: "This is the name of the node. It should always be unique in your ESPHome network. May only contain lowercase characters, digits and hyphens",
                    const.CONF_VAR_GETTER: lambda c, _: c[CONF_NAME],
                    const.CONF_VAR_SETTER: lambda c, o: _node_setter(c, o),
                },
                KEY_NODE_NAME_ADD_MAC_SUFFIX: {
                    const.CONF_VAR_TYPE: var.VT_BOOL,
                    const.CONF_VAR_NAME: "Add MAC-address suffix to node name",
                    const.CONF_VAR_HELP: "Appends the last 3 bytes of the mac address of the device to the name in the form <node_name>-aabbcc",
                    const.CONF_VAR_GETTER: lambda c, _: c[CONF_NAME_ADD_MAC_SUFFIX],
                },
            }
        ),
        PRESET_WIFI: OrderedDict(
            {
                KEY_WIFI_SSID: {
                    const.CONF_VAR_TYPE: var.VT_STR,
                    const.CONF_VAR_MIN: 1,
                    const.CONF_VAR_NAME: "SSID",
                    const.CONF_VAR_HELP: "The name of the WiFi access point your device should connect to",
                    const.CONF_VAR_GETTER: lambda c, _: _wifi_network_getter(
                        0, c, wifi.CONF_SSID
                    ),
                },
                KEY_WIFI_PASSWORD: {
                    const.CONF_VAR_TYPE: var.VT_PASSWORD,
                    const.CONF_VAR_NAME: "Password",
                    const.CONF_VAR_HELP: "The password (or PSK) for your WiFi network. Leave empty for no password",
                    const.CONF_VAR_GETTER: lambda c, _: _wifi_network_getter(
                        0, c, wifi.CONF_PASSWORD
                    ),
                    const.CONF_VAR_SETTER: lambda c, o: _wifi_network_setter(0, c, o),
                },
                "wifi_btm": {
                    const.CONF_VAR_TYPE: var.VT_BOOL,
                    const.CONF_VAR_NAME: "Enable BTM",
                    const.CONF_VAR_HELP: "Enable 802.11v BSS Transition Management support",
                    const.CONF_VAR_IFDEF: [USE_WIFI, "USE_WIFI_11KV_SUPPORT"],
                    const.CONF_VAR_GETTER: lambda c, _: c.get(
                        wifi.CONF_ENABLE_BTM, False
                    ),
                    const.CONF_VAR_SETTER: lambda _, o: o.set_btm,
                },
                "wifi_rrm": {
                    const.CONF_VAR_TYPE: var.VT_BOOL,
                    const.CONF_VAR_NAME: "Enable RRM",
                    const.CONF_VAR_HELP: "Enable 802.11k Radio Resource Management support",
                    const.CONF_VAR_IFDEF: [USE_WIFI, "USE_WIFI_11KV_SUPPORT"],
                    const.CONF_VAR_GETTER: lambda c, _: c.get(
                        wifi.CONF_ENABLE_RRM, False
                    ),
                    const.CONF_VAR_SETTER: lambda _, o: o.set_rrm,
                },
            }
        ),
        PRESET_API: OrderedDict(
            {
                "api_reboot_timeout": {
                    const.CONF_VAR_TYPE: var.VT_TIMEOUT,
                    const.CONF_VAR_HELP: "The amount of time in seconds to wait before rebooting when no API connection exists",
                    const.CONF_VAR_GETTER: lambda c, _: c[api.CONF_REBOOT_TIMEOUT],
                    const.CONF_VAR_SETTER: lambda _, o: o.set_reboot_timeout,
                },
            }
        ),
        PRESET_MQTT: OrderedDict(
            {
                "mqtt_host": {
                    const.CONF_VAR_TYPE: var.VT_STR,
                    const.CONF_VAR_MIN: 1,
                    const.CONF_VAR_HELP: "The host of your MQTT broker",
                    const.CONF_VAR_GETTER: lambda c, _: c[mqtt.CONF_BROKER],
                    const.CONF_VAR_SETTER: lambda _, o: o.set_broker_address,
                },
                "mqtt_port": {
                    const.CONF_VAR_TYPE: var.VT_UINT16,
                    const.CONF_VAR_HELP: "The port to connect to",
                    const.CONF_VAR_GETTER: lambda c, _: c[mqtt.CONF_PORT],
                    const.CONF_VAR_SETTER: lambda _, o: o.set_broker_port,
                },
                "mqtt_username": {
                    const.CONF_VAR_TYPE: var.VT_STR,
                    const.CONF_VAR_HELP: "The username to use for authentication. Empty means no authentication",
                    const.CONF_VAR_GETTER: lambda c, _: c[mqtt.CONF_USERNAME],
                    const.CONF_VAR_SETTER: lambda _, o: o.set_username,
                },
                "mqtt_password": {
                    const.CONF_VAR_TYPE: var.VT_PASSWORD,
                    const.CONF_VAR_HELP: "The password to use for authentication. Empty means no authentication",
                    const.CONF_VAR_GETTER: lambda c, _: c[mqtt.CONF_PASSWORD],
                    const.CONF_VAR_SETTER: lambda _, o: o.set_password,
                },
                "mqtt_client_id": {
                    const.CONF_VAR_TYPE: var.VT_STR,
                    const.CONF_VAR_HELP: "The client id to use for opening connections. If empty the client id will automatically be generated by using your node name and adding the MAC address of your device to it",
                    const.CONF_VAR_MAX: 23,
                    const.CONF_VAR_GETTER: lambda c, _: c.get(mqtt.CONF_CLIENT_ID, ""),
                    const.CONF_VAR_SETTER: lambda _, o: o.set_client_id,
                },
                "mqtt_topic_prefix": {
                    const.CONF_VAR_TYPE: var.VT_STR,
                    const.CONF_VAR_HELP: "The prefix used for all MQTT messages. Set to empty to disable publishing or subscribing of any MQTT topic unless it is explicitly configured",
                    const.CONF_VAR_GETTER: lambda _, o: _mqtt_access(o).topic_prefix(),
                    const.CONF_VAR_SETTER: lambda _, o: _mqtt_access(o).topic_prefix,
                },
                "mqtt_birth_message": {
                    const.CONF_VAR_TYPE: var.VT_STR,
                    const.CONF_VAR_HELP: "The message to send when a connection to the broker is established",
                    const.CONF_VAR_GETTER: lambda _, o: _mqtt_access(
                        o
                    ).birth_message_topic(),
                    const.CONF_VAR_SETTER: lambda _, o: _mqtt_access(
                        o
                    ).birth_message_topic,
                },
                "mqtt_will_message": {
                    const.CONF_VAR_TYPE: var.VT_STR,
                    const.CONF_VAR_HELP: "The message to send when the MQTT connection is dropped",
                    const.CONF_VAR_GETTER: lambda _, o: _mqtt_access(
                        o
                    ).last_will_topic(),
                    const.CONF_VAR_SETTER: lambda _, o: _mqtt_access(o).last_will_topic,
                },
                "mqtt_shutdown_message": {
                    const.CONF_VAR_TYPE: var.VT_STR,
                    const.CONF_VAR_HELP: "The message to send when the node shuts down and the connection is closed cleanly",
                    const.CONF_VAR_GETTER: lambda _, o: _mqtt_access(
                        o,
                    ).shutdown_message_topic(),
                    const.CONF_VAR_SETTER: lambda _, o: _mqtt_access(
                        o
                    ).shutdown_message_topic,
                },
                "mqtt_log_topic": {
                    const.CONF_VAR_TYPE: var.VT_STR,
                    const.CONF_VAR_HELP: " The topic to send MQTT log messages to. Set to empty for disable logging",
                    const.CONF_VAR_GETTER: lambda _, o: _mqtt_access(
                        o
                    ).log_message_topic(),
                    const.CONF_VAR_SETTER: lambda _, o: _mqtt_access(
                        o
                    ).log_message_topic,
                },
                "mqtt_reboot_timeout": {
                    const.CONF_VAR_TYPE: var.VT_TIMEOUT,
                    const.CONF_VAR_HELP: "The amount of time in seconds to wait before rebooting when no MQTT connection exists",
                    const.CONF_VAR_GETTER: lambda c, _: c[mqtt.CONF_REBOOT_TIMEOUT],
                    const.CONF_VAR_SETTER: lambda _, o: o.set_reboot_timeout,
                },
            }
        ),
        PRESET_WEB_SERVER: OrderedDict(
            {
                "web_server_username": {
                    const.CONF_VAR_TYPE: var.VT_STR,
                    const.CONF_VAR_HELP: "The username to use for authentication. Set to empty for remove authentication",
                    const.CONF_VAR_GETTER: lambda c, _: c[web_server.CONF_AUTH][
                        web_server.CONF_USERNAME
                    ]
                    if web_server.CONF_AUTH in c
                    else "",
                    const.CONF_VAR_SETTER: lambda c, _: cg.MockObj(
                        c[web_server.CONF_WEB_SERVER_BASE_ID], "->"
                    ).set_auth_username,
                },
                "web_server_password": {
                    const.CONF_VAR_TYPE: var.VT_PASSWORD,
                    const.CONF_VAR_HELP: "The password to check for authentication",
                    const.CONF_VAR_GETTER: lambda c, _: c[web_server.CONF_AUTH][
                        web_server.CONF_PASSWORD
                    ]
                    if web_server.CONF_AUTH in c
                    else "",
                    const.CONF_VAR_SETTER: lambda c, _: cg.MockObj(
                        c[web_server.CONF_WEB_SERVER_BASE_ID], "->"
                    ).set_auth_password,
                },
                "web_server_log": {
                    const.CONF_VAR_TYPE: var.VT_BOOL,
                    const.CONF_VAR_HELP: "Turn on or off the log feature inside web server",
                    const.CONF_VAR_GETTER: lambda c, _: c[web_server.CONF_LOG],
                    const.CONF_VAR_SETTER: lambda _, o: o.set_expose_log,
                },
                "web_server_ota": {
                    const.CONF_VAR_IFDEF: [USE_WEB_SERVER, USE_ARDUINO],
                    const.CONF_VAR_TYPE: var.VT_BOOL,
                    const.CONF_VAR_NAME: "OTA",
                    const.CONF_VAR_HELP: "Turn on or off the OTA feature inside web server. Strongly not suggested without enabled authentication settings",
                    const.CONF_VAR_GETTER: lambda c, _: c[web_server.CONF_OTA],
                    const.CONF_VAR_SETTER: lambda _, o: o.set_allow_ota,
                },
            }
        ),
    }
)

PRESETS = list(_PRESETS.keys())


def _add_load_nvs_var(nvs: cg.MockObj, v: str, type: cg.MockObj, ss: list):
    v = var.Var(v, {})
    name = cg.MockObj(v.id)
    ss.append(cgp.ConstAutoAssignmentExpression(name, nvs.get.template(type)(v.key)))
    return name


def _add_load_nvs_str_var(nvs: cg.MockObj, v: str, ss: list):
    return _add_load_nvs_var(nvs, v, cg.std_string, ss)


def _node_setter(config, App: cg.MockObj):
    nvs = cg.MockObj("nvs")

    ss = []

    node_name = _add_load_nvs_str_var(nvs, KEY_NODE_NAME, ss)
    node_name_add_mac_suffix = _add_load_nvs_var(
        nvs, KEY_NODE_NAME_ADD_MAC_SUFFIX, cg.bool_, ss
    )
    ss.append(
        cgp.IfStatement(
            f"{node_name.has_value()} || {node_name_add_mac_suffix.has_value()}",
            [
                cgp.IfStatement(
                    node_name.has_value(),
                    cpp.log_expression(KEY_NODE_NAME, var.VT_STR, node_name.value()),
                ),
                cgp.IfStatement(
                    node_name_add_mac_suffix.has_value(),
                    cpp.log_expression(
                        KEY_NODE_NAME_ADD_MAC_SUFFIX,
                        var.VT_BOOL,
                        node_name_add_mac_suffix.value(),
                    ),
                ),
                App.pre_setup(
                    cgp.ConditionalExperession(
                        node_name.has_value(),
                        node_name.value(),
                        config[CONF_NAME],
                    ),
                    config[CONF_FRIENDLY_NAME],
                    config[CONF_AREA],
                    config.get(CONF_COMMENT, ""),
                    cg.RawExpression('__DATE__ ", " __TIME__'),
                    cgp.ConditionalExperession(
                        node_name_add_mac_suffix.has_value(),
                        node_name_add_mac_suffix.value(),
                        config[CONF_NAME_ADD_MAC_SUFFIX],
                    ),
                ),
            ],
        )
    )

    return cgp.StatementList(ss)


def _wifi_network_getter(index: int, c: dict, key: str) -> str:
    networks = c.get(wifi.CONF_NETWORKS, [])
    if len(networks) > index:
        return networks[index][key]
    return None


def _wifi_network_setter(index: int, c: dict, wifi_comp: cg.MockObj) -> cg.Statement:
    nvs = cg.MockObj("nvs")

    ss = []

    wifi_ssid = _add_load_nvs_str_var(nvs, KEY_WIFI_SSID, ss)
    wifi_password = _add_load_nvs_str_var(nvs, KEY_WIFI_PASSWORD, ss)

    sta = cg.MockObj("wifi_sta")
    ss.append(
        cgp.IfStatement(
            f"{wifi_ssid.has_value()} || {wifi_password.has_value()}",
            [
                cgp.IfStatement(
                    wifi_ssid.has_value(),
                    cpp.log_expression(KEY_WIFI_SSID, var.VT_STR, wifi_ssid.value()),
                ),
                cgp.IfStatement(
                    wifi_password.has_value(),
                    cpp.log_expression(
                        KEY_WIFI_PASSWORD, var.VT_PASSWORD, wifi_password.value()
                    ),
                ),
                cg.RawExpression(f"{wifi.WiFiAP} {sta}{{}}"),
                sta.set_ssid(
                    cgp.ConditionalExperession(
                        wifi_ssid.has_value(),
                        wifi_ssid.value(),
                        _wifi_network_getter(index, c, wifi.CONF_SSID),
                    )
                ),
                sta.set_password(
                    cgp.ConditionalExperession(
                        wifi_password.has_value(),
                        wifi_password.value(),
                        _wifi_network_getter(index, c, wifi.CONF_PASSWORD),
                    )
                ),
                wifi_comp.set_sta(sta),
            ],
        )
    )

    return cgp.CodeBlock(None, None, ss)


def _mqtt_access(mqtt_comp: cg.MockObj):
    return cg.MockObj("settings::mqtt_access", "->")(mqtt_comp)


def _get_presets(cpresets: list[str]) -> OrderedDict[str, Any]:
    presets = OrderedDict()

    for name in cpresets:
        if name not in _PRESETS:
            continue
        preset = _PRESETS[name]
        for key, val in preset.items():
            cpy = _PRESET_DEFAULTS[name].copy()
            cpy.update(val)
            val = cpy
            if val[const.CONF_VAR_COMP] in CORE.config:
                presets[key] = val

    return presets


def presets_init(cpresets: list[str], vars: dict[str, dict[str, Any]]):
    for key, val in _get_presets(cpresets).items():
        val.update(vars.get(key, {}))
        vars[key] = val

    if PRESET_WIFI in cpresets and CORE.using_esp_idf:
        esp32.add_idf_sdkconfig_option("CONFIG_WPA_11KV_SUPPORT", True)
        cg.add_define("USE_WIFI_11KV_SUPPORT")
        node_name = CORE.config[COMP_ESPHOME][CONF_NAME]
        use_address = node_name + CORE.config[COMP_WIFI][wifi.CONF_DOMAIN]
        if use_address == CORE.config[COMP_WIFI][wifi.CONF_USE_ADDRESS]:
            wifi_component = cgp.MockObjPtr(CORE.config[COMP_WIFI][CONF_ID])
            # reset hardcoded use_address to use auto expanded one
            cg.add(wifi_component.set_use_address(""))
