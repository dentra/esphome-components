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
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CARBON_DIOXIDE,
    DEVICE_CLASS_DOOR,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_ILLUMINANCE,
    DEVICE_CLASS_LIGHT,
    DEVICE_CLASS_MOTION,
    DEVICE_CLASS_PM10,
    DEVICE_CLASS_PM25,
    DEVICE_CLASS_PRESSURE,
    DEVICE_CLASS_PROBLEM,
    DEVICE_CLASS_TEMPERATURE,
    UNIT_CELSIUS,
    UNIT_HECTOPASCAL,
    UNIT_LUX,
    UNIT_MICROGRAMS_PER_CUBIC_METER,
    UNIT_PARTS_PER_MILLION,
    UNIT_PERCENT,
)
from esphome.const import __version__ as ESPHOME_VERSION
from esphome.core import CORE, MACAddress

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
CONF_MAX_DEVICES = "max_devices"

KEY_PLATFORM_SENSOR = "sensor"
KEY_PLATFORM_BINARY_SENSOR = "binary_sensor"


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
        cv.Optional(CONF_MAX_DEVICES, default=5): cv.int_range(min=1, max=64),
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
        raise cv.Invalid(f"{CONF_MAC_ADDRESS} is required")

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
        # check for only one explorer without mac-address
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


def _entity_fields(device_class: str, uom: str | None) -> int:
    """Packed entity fields for ESPHome 2026.3.0 and up, 0 for older versions."""
    if cv.Version.parse(ESPHOME_VERSION) < cv.Version.parse("2026.3.0"):
        return 0
    from esphome.core import entity_helpers

    fields = entity_helpers.register_device_class(device_class)
    if uom:
        fields |= entity_helpers.register_unit_of_measurement(uom) << 8
    return fields


async def _setup_explorer(config: dict, parent, has_mac: bool):
    var = cg.new_Pvariable(config[CONF_ID], parent)
    await cg.register_component(var, config)

    # hub with mac-address listens only one device
    max_devices = 1 if has_mac else config[CONF_MAX_DEVICES]
    cg.add(var.set_max_devices(max_devices))

    for key, pla, device_class, uom in [
        (CONF_TEMPERATURE, KEY_PLATFORM_SENSOR, DEVICE_CLASS_TEMPERATURE, UNIT_CELSIUS),
        (CONF_HUMIDITY, KEY_PLATFORM_SENSOR, DEVICE_CLASS_HUMIDITY, UNIT_PERCENT),
        (CONF_BATTERY_LEVEL, KEY_PLATFORM_SENSOR, DEVICE_CLASS_BATTERY, UNIT_PERCENT),
        (CONF_DOOR, KEY_PLATFORM_BINARY_SENSOR, DEVICE_CLASS_DOOR, None),
        (CONF_DOOR_LEFT_OPEN, KEY_PLATFORM_BINARY_SENSOR, DEVICE_CLASS_PROBLEM, None),
        (CONF_PRESSURE, KEY_PLATFORM_SENSOR, DEVICE_CLASS_PRESSURE, UNIT_HECTOPASCAL),
        (CONF_MOTION, KEY_PLATFORM_BINARY_SENSOR, DEVICE_CLASS_MOTION, None),
        (CONF_ILLUMINANCE, KEY_PLATFORM_SENSOR, DEVICE_CLASS_ILLUMINANCE, UNIT_LUX),
        (CONF_LIGHT, KEY_PLATFORM_BINARY_SENSOR, DEVICE_CLASS_LIGHT, None),
        (CONF_PM10, KEY_PLATFORM_SENSOR, DEVICE_CLASS_PM10, UNIT_MICROGRAMS_PER_CUBIC_METER),
        (CONF_PM25, KEY_PLATFORM_SENSOR, DEVICE_CLASS_PM25, UNIT_MICROGRAMS_PER_CUBIC_METER),
        (CONF_CO2, KEY_PLATFORM_SENSOR, DEVICE_CLASS_CARBON_DIOXIDE, UNIT_PARTS_PER_MILLION),
    ]:
        if value := config.get(key, None):
            fields = _entity_fields(device_class, uom)
            cg.add(getattr(var, f"set_name_{key}")(value, fields))
            # reserve entity slots for each device
            for _ in range(max_devices):
                CORE.register_platform_component(pla, None)

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
        await _setup_explorer(config[CONF_EXPLORER], var, CONF_MAC_ADDRESS in config)

    await cgp.setup_automation(config, CONF_ON_STATE, var, (QPDataPointRef, "x"))


async def to_code(config):
    """Code generation entry point"""
    macs = [conf[CONF_MAC_ADDRESS] for conf in config if CONF_MAC_ADDRESS in conf]
    for conf in config:
        await _setup_hub(conf, macs)


def new_pc(pc_cfg: dict[str, str | dict[str, Any]]):
    return cgp.PC(Qingping, CONF_QINGPING_ID, pc_cfg)


def qp_final_validate_config(config):
    full_config = fv.full_config.get()
    for conf in full_config["qingping"]:
        if conf[CONF_ID] == config[CONF_QINGPING_ID] and CONF_MAC_ADDRESS not in conf:
            raise cv.Invalid("Required hub with MAC-address")
    return config
