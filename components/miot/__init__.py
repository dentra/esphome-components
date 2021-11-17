import logging
from esphome.cpp_types import Component
from esphome.core import CORE, Lambda
from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, esp32_ble_tracker, sensor, text_sensor
from esphome.const import (
    CONF_BATTERY_LEVEL,
    CONF_BATTERY_VOLTAGE,
    CONF_BINDKEY,
    CONF_ID,
    CONF_LAMBDA,
    CONF_MAC_ADDRESS,
    CONF_NAME,
    CONF_PASSWORD,
    CONF_SERVERS,
    CONF_THEN,
    CONF_TRIGGER_ID,
    CONF_UPDATE_INTERVAL,
    CONF_USERNAME,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    PLATFORM_ESP32,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
    UNIT_VOLT,
)
from .xiaomi_beaconkeys import XiaomiBeaconkeys

CODEOWNERS = ["@dentra"]
ESP_PLATFORMS = [PLATFORM_ESP32]
AUTO_LOAD = ["esp32_ble_tracker", "sensor"]

CONF_MIOT_ID = "miot_id"
CONF_ON_MIOT_ADVERTISE = "on_miot_advertise"
CONF_PRODUCT_ID = "product_id"
CONF_RSSI = "rssi"
CONF_DEBUG = "debug"
CONF_XIAOMI_ACCOUNT = "xiaomi_account"

DEFAULT_SERVERS = ["cn", "de", "us", "ru", "tw", "sg", "in", "i2"]

_LOGGER = logging.getLogger(__name__)

miot_ns = cg.esphome_ns.namespace("miot")
MiBeaconTracker = miot_ns.class_(
    "MiBeaconTracker", esp32_ble_tracker.ESPBTDeviceListener, Component
)
MiotListener = miot_ns.class_("MiotListener")
MiotComponent = miot_ns.class_("MiotComponent", MiotListener, Component)
BLEObjectConstRef = miot_ns.class_("BLEObject").operator("ref").operator("const")
MiotAdvertiseTrigger = miot_ns.class_(
    "MiotAdvertiseTrigger", automation.Trigger.template(BLEObjectConstRef)
)


def cv_bind_key(value):
    value = cv.string_strict(value)
    parts = [value[i : i + 2] for i in range(0, len(value), 2)]
    if len(parts) != 16 and len(parts) != 12:
        raise cv.Invalid("Bind key must consist of 16 or 12 hexadecimal numbers")
    parts_int = []
    if any(len(part) != 2 for part in parts):
        raise cv.Invalid("Bind key must be format XX")
    for part in parts:
        try:
            parts_int.append(int(part, 16))
        except ValueError:
            # pylint: disable=raise-missing-from
            raise cv.Invalid("Bind key must be hex values from 00 to FF")
    if len(parts_int) == 12:
        parts_int.extend([0xFF, 0xFF, 0xFF, 0xFF])

    return "".join(f"{part:02X}" for part in parts_int)


MIOT_BLE_DEVICE_CORE_SCHEMA = (
    cv.Schema(
        {
            cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
            cv.Optional(CONF_BINDKEY): cv_bind_key,
        }
    )
    .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MiBeaconTracker),
            cv.Optional(CONF_XIAOMI_ACCOUNT): cv.Schema(
                {
                    cv.Required(CONF_USERNAME): cv.string_strict,
                    cv.Required(CONF_PASSWORD): cv.string_strict,
                    cv.Optional(CONF_UPDATE_INTERVAL, default="1d"): cv.update_interval,
                    cv.Optional(CONF_SERVERS, default=DEFAULT_SERVERS): cv.All(
                        cv.ensure_list(cv.one_of(*DEFAULT_SERVERS, lower=True)),
                        cv.Length(min=1),
                    ),
                }
            ),
            cv.Optional(CONF_ON_MIOT_ADVERTISE): automation.validate_automation(
                cv.Schema(
                    {
                        cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
                            MiotAdvertiseTrigger
                        ),
                        cv.Optional(CONF_PRODUCT_ID): cv.uint16_t,
                        cv.Optional(CONF_DEBUG): cv.boolean,
                        cv.Optional(
                            CONF_THEN, default={CONF_LAMBDA: Lambda("")}
                        ): automation.validate_action_list,
                    }
                ).extend(MIOT_BLE_DEVICE_CORE_SCHEMA),
            ),
        }
    )
    .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

MIOT_BLE_DEVICE_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MIOT_ID): cv.use_id(MiBeaconTracker),
        cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_BATTERY,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
).extend(MIOT_BLE_DEVICE_CORE_SCHEMA)


def as_bindkey(value):
    cpp_array = [
        f"0x{part}" for part in [value[i : i + 2] for i in range(0, len(value), 2)]
    ]
    return cg.RawExpression("(const uint8_t[16]){{{}}}".format(",".join(cpp_array)))


async def register_miot_device(var, config):
    parent = await cg.get_variable(config[CONF_MIOT_ID])
    cg.add(parent.register_listener(var))
    return var


async def setup_device_core_(var, config):
    cg.add(var.set_address(config[CONF_MAC_ADDRESS].as_hex))
    if CONF_BINDKEY in config:
        cg.add(var.set_bindkey(as_bindkey(config[CONF_BINDKEY])))
    else:
        conf = CORE.config["miot"].get(CONF_XIAOMI_ACCOUNT)
        if conf:
            xbk = XiaomiBeaconkeys(
                username=conf[CONF_USERNAME],
                password=conf[CONF_PASSWORD],
                servers=conf[CONF_SERVERS],
                storage_path=CORE.build_path,
                update_interval=conf[CONF_UPDATE_INTERVAL].total_seconds,
            )
            bindkey = xbk.get_beaconkey(config[CONF_MAC_ADDRESS])
            if bindkey:
                _LOGGER.info(
                    "Got bindkey for %s %s",
                    config[CONF_MAC_ADDRESS],
                    config.get(CONF_NAME, ""),
                )
                cg.add(var.set_bindkey(as_bindkey(bindkey)))


async def new_device(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await register_miot_device(var, config)

    await setup_device_core_(var, config)
    if CONF_BATTERY_LEVEL in config:
        conf = config[CONF_BATTERY_LEVEL]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_battery_level(sens))
    if CONF_BATTERY_VOLTAGE in config:
        conf = config[CONF_BATTERY_VOLTAGE]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_battery_voltage(sens))

    return var


async def new_sensor_device(config):
    var = await new_device(config)
    await sensor.register_sensor(var, config)
    return var


async def new_binary_sensor_device(config):
    var = await new_device(config)
    await binary_sensor.register_binary_sensor(var, config)
    return var


async def new_text_sensor_device(config):
    var = await new_device(config)
    await text_sensor.register_text_sensor(var, config)
    return var


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await esp32_ble_tracker.register_ble_device(var, config)

    for conf in config.get(CONF_ON_MIOT_ADVERTISE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID])
        cg.add(var.register_listener(trigger))
        await setup_device_core_(trigger, conf)
        if CONF_PRODUCT_ID in conf:
            cg.add(trigger.set_product_id(conf[CONF_PRODUCT_ID]))
        if conf.get(CONF_DEBUG, False):
            cg.add(trigger.set_debug(True))
        await automation.build_automation(trigger, [(BLEObjectConstRef, "x")], conf)
