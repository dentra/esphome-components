import logging
from esphome.cpp_types import Component
from esphome.core import Lambda
from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, esp32_ble_tracker, sensor, text_sensor
from esphome.const import (
    CONF_BATTERY_LEVEL,
    CONF_BATTERY_VOLTAGE,
    CONF_ID,
    CONF_LAMBDA,
    CONF_MAC_ADDRESS,
    CONF_THEN,
    CONF_TRIGGER_ID,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    PLATFORM_ESP32,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
    UNIT_VOLT,
)
from .. import xiaomi_account

CODEOWNERS = ["@dentra"]
ESP_PLATFORMS = [PLATFORM_ESP32]
AUTO_LOAD = ["esp32_ble_tracker", "sensor"]

CONF_MIOT_ID = "miot_id"
CONF_ON_MIOT_ADVERTISE = "on_miot_advertise"
CONF_PRODUCT_ID = "product_id"
CONF_RSSI = "rssi"
CONF_DEBUG = "debug"

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


MIOT_BLE_DEVICE_CORE_SCHEMA = (
    cv.Schema(
        {
            cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
        }
    )
    .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
    .extend(xiaomi_account.BINDKEY_SCHEMA)
)


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MiBeaconTracker),
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
    .extend(xiaomi_account.XIAOMI_ACCOUNT_SCHEMA)
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


async def register_miot_device(var, config):
    parent = await cg.get_variable(config[CONF_MIOT_ID])
    cg.add(parent.register_listener(var))
    return var


async def setup_device_core_(var, config):
    cg.add(var.set_address(config[CONF_MAC_ADDRESS].as_hex))
    xiaomi_account.set_bindkey(config, config[CONF_MAC_ADDRESS], var.set_bindkey)


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
