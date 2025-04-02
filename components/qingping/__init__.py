from typing import Any

import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome import automation
from esphome.components import esp32_ble_tracker
from esphome.const import (
    CONF_BATTERY_LEVEL,
    CONF_CO2,
    CONF_HUMIDITY,
    CONF_ID,
    CONF_ILLUMINANCE,
    CONF_LIGHT,
    CONF_MAC_ADDRESS,
    CONF_MOTION,
    CONF_ON_STATE,
    CONF_PRESSURE,
    CONF_TEMPERATURE,
)
from esphome.core import MACAddress

from .. import cgp  # pylint: disable=relative-beyond-top-level

CODEOWNERS = ["@dentra"]
AUTO_LOAD = [
    "esp32_ble_tracker",
    "cgp",
    "sensor",  # required by explorer
    "binary_sensor",  # required by explorer
]

qingping_ns = cg.esphome_ns.namespace("qingping")
qingping_pc_ns = qingping_ns.namespace("property_controller")
Qingping = qingping_ns.class_("Qingping", cg.Component)
QPDataPointRef = qingping_ns.class_("QPDataPoint").operator("ref").operator("const")
QingpingStateTrigger = qingping_ns.class_(
    "QingpingStateTrigger", automation.Trigger.template(QPDataPointRef)
)
QingpingExplorer = qingping_ns.class_("QingpingExplorer", cg.Component)

CONF_USE_BEACON_MAC = "use_beacon_mac"
CONF_VERBOSE = "verbose"
CONF_EXPLORER = "explorer"
CONF_QINGPING_ID = "qingping_id"
CONF_DOOR = "door"
CONF_DOOR_LEFT_OPEN = "door_left_open"
CONF_PM10 = "pm10"
CONF_PM25 = "pm25"

string_or_none = cv.Any(cv.boolean_false, cv.none, cv.string)

EXPLORER_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(QingpingExplorer),
        cv.Optional(CONF_TEMPERATURE, default="Temperature"): string_or_none,
        cv.Optional(CONF_HUMIDITY, default="Humidity"): string_or_none,
        cv.Optional(CONF_BATTERY_LEVEL, default="Battery Level"): string_or_none,
        cv.Optional(CONF_DOOR, default="Door"): string_or_none,
        cv.Optional(CONF_DOOR_LEFT_OPEN, default="Door Left Open"): string_or_none,
        cv.Optional(CONF_PRESSURE, default="Pressure"): string_or_none,
        cv.Optional(CONF_MOTION, default="Motion"): string_or_none,
        cv.Optional(CONF_ILLUMINANCE, default="Illuminance"): string_or_none,
        cv.Optional(CONF_LIGHT, default="Light"): string_or_none,
        cv.Optional(CONF_PM10, default="PM 10"): string_or_none,
        cv.Optional(CONF_PM25, default="PM 2.5"): string_or_none,
        cv.Optional(CONF_CO2, default="Carbon Dioxide"): string_or_none,
    }
)


def _check_hub():
    def validator(config):
        if CONF_MAC_ADDRESS in config:
            return config
        # allow any mac with explorer
        if CONF_EXPLORER in config:
            return config
        # allow any mac with verbose mode
        if config.get(CONF_VERBOSE, False):
            return config
        raise cv.Invalid(f"{CONF_MAC_ADDRESS} is requred")

    return validator


def _check_final():
    def validator(config):
        # check for unique mac-address
        macs = []
        for conf in config:
            if CONF_MAC_ADDRESS not in conf:
                continue
            mac = str(conf[CONF_MAC_ADDRESS])
            if mac in macs:
                raise cv.Invalid(f"Duplicate MAC-address: {mac}")
            macs.append(mac)
        # check for only one explorer wuthout mac-address
        any_explorer_count = 0
        for conf in config:
            if CONF_EXPLORER not in conf and not conf.get(CONF_VERBOSE, False):
                continue
            if CONF_MAC_ADDRESS in conf:
                continue
            any_explorer_count += 1
        if any_explorer_count > 1:
            raise cv.Invalid("Only one hub without mac address is allowed")
        return config

    return validator


CONFIG_SCHEMA = cv.All(
    cv.ensure_list(
        cv.Schema(
            {
                cv.GenerateID(): cv.declare_id(Qingping),
                cv.Optional(CONF_MAC_ADDRESS): cv.mac_address,
                cv.Optional(CONF_USE_BEACON_MAC): cv.boolean,
                cv.Optional(CONF_VERBOSE): cv.boolean,
                cv.Optional(CONF_EXPLORER): EXPLORER_SCHEMA,
                cv.Optional(CONF_ON_STATE): cgp.automation_schema(QingpingStateTrigger),
            }
        )
        .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
        .extend(cv.COMPONENT_SCHEMA),
        _check_hub(),
    ),
    _check_final(),
)


async def _setup_explorer_name(config: dict, key: str, var: cg.MockObj):
    if value := config.get(key, None):
        cg.add(getattr(var, f"set_name_{key}")(value))


async def _setup_explorer(config, parent):
    var = cg.new_Pvariable(config[CONF_ID], parent)
    await cg.register_component(var, config)

    await _setup_explorer_name(config, CONF_TEMPERATURE, var)
    await _setup_explorer_name(config, CONF_HUMIDITY, var)
    await _setup_explorer_name(config, CONF_BATTERY_LEVEL, var)
    await _setup_explorer_name(config, CONF_DOOR, var)
    await _setup_explorer_name(config, CONF_DOOR_LEFT_OPEN, var)
    await _setup_explorer_name(config, CONF_PRESSURE, var)
    await _setup_explorer_name(config, CONF_MOTION, var)
    await _setup_explorer_name(config, CONF_ILLUMINANCE, var)
    await _setup_explorer_name(config, CONF_LIGHT, var)
    await _setup_explorer_name(config, CONF_PM10, var)
    await _setup_explorer_name(config, CONF_PM25, var)
    await _setup_explorer_name(config, CONF_CO2, var)

    cg.add_define("USE_QINGPING_EXPLORER")


async def _setup_hub(config: dict, macs: list[MACAddress]):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await esp32_ble_tracker.register_ble_device(var, config)

    cgp.setup_value(config, CONF_VERBOSE, var.set_verbose)

    if CONF_MAC_ADDRESS in config:
        cg.add(var.set_address(config[CONF_MAC_ADDRESS].as_hex))
    elif macs:
        cg.add(var.set_mac_exclude([mac.as_hex for mac in macs]))

    cgp.setup_value(config, CONF_USE_BEACON_MAC, var.set_use_beacon_mac, False)

    if CONF_EXPLORER in config:
        await _setup_explorer(config[CONF_EXPLORER], var)

    await cgp.setup_automation(config, CONF_ON_STATE, var, (QPDataPointRef, "x"))


async def to_code(config):
    """Code generation entry point"""
    macs = [conf[CONF_MAC_ADDRESS] for conf in config if CONF_MAC_ADDRESS in conf]
    for conf in config:
        await _setup_hub(conf, macs)

    cg.add_build_flag("-std=gnu++17")
    cg.add_platformio_option("build_unflags", ["-std=gnu++11"])


def new_pc(pc_cfg: dict[str, str | dict[str, Any]]):
    return cgp.PC(Qingping, CONF_QINGPING_ID, pc_cfg)


def qp_final_validate_config(config):
    full_config = fv.full_config.get()
    for conf in full_config["qingping"]:
        if conf[CONF_ID] == config[CONF_QINGPING_ID] and CONF_MAC_ADDRESS not in conf:
            raise cv.Invalid("Required hub with MAC-address")
    return config
