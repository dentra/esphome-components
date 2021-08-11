from esphome.cpp_types import Component
from esphome.core import ID
from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, esp32_ble_tracker, sensor
from esphome.const import (
    CONF_BATTERY_LEVEL,
    CONF_BINDKEY,
    CONF_ID,
    CONF_MAC_ADDRESS,
    CONF_ON_BLE_SERVICE_DATA_ADVERTISE,
    CONF_TRIGGER_ID,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_SIGNAL_STRENGTH,
    ESP_PLATFORM_ESP32,
    ESP_PLATFORM_ESP8266,
    ICON_BATTERY,
    ICON_EMPTY,
    STATE_CLASS_MEASUREMENT,
    UNIT_DECIBEL,
    UNIT_PERCENT,
)

CODEOWNERS = ["@dentra"]
ESP_PLATFORMS = [ESP_PLATFORM_ESP32]
AUTO_LOAD = ["esp32_ble_tracker"]

CONF_ON_MIOT_ADVERTISE = "on_miot_advertise"
CONF_PRODUCT_ID = "product_id"
CONF_RSSI = "rssi"
CONF_DEBUG = "debug"

miot_ns = cg.esphome_ns.namespace("miot")
MiotListener = miot_ns.class_("MiotListener", esp32_ble_tracker.ESPBTDeviceListener)
MiotComponent = miot_ns.class_("MiotComponent", MiotListener, Component)
BLEObjectConstRef = miot_ns.class_("BLEObject").operator("ref").operator("const")
MiotAdvertiseTrigger = miot_ns.class_(
    "MiotAdvertiseTrigger", automation.Trigger.template(BLEObjectConstRef)
)

MIOT_BLE_DEVICE_CORE_SCHEMA = (
    cv.Schema(
        {
            cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
            cv.Optional(CONF_BINDKEY): cv.bind_key,
        }
    )
    .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(MiotListener),
        cv.Optional(CONF_ON_MIOT_ADVERTISE): automation.validate_automation(
            cv.Schema(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotAdvertiseTrigger),
                    cv.Optional(CONF_PRODUCT_ID): cv.uint32_t,
                    cv.Optional(CONF_DEBUG): cv.boolean,
                }
            ).extend(MIOT_BLE_DEVICE_CORE_SCHEMA),
        ),
    }
).extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)

MIOT_BLE_DEVICE_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
            UNIT_PERCENT,
            ICON_BATTERY,
            0,
            DEVICE_CLASS_BATTERY,
            STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_RSSI): sensor.sensor_schema(
            UNIT_DECIBEL,
            ICON_EMPTY,
            0,
            DEVICE_CLASS_SIGNAL_STRENGTH,
            STATE_CLASS_MEASUREMENT,
        ),
    }
).extend(MIOT_BLE_DEVICE_CORE_SCHEMA)


def as_bindkey(value):
    cpp_array = [
        f"0x{part}" for part in [value[i : i + 2] for i in range(0, len(value), 2)]
    ]
    return cg.RawExpression("(const uint8_t[16]){{{}}}".format(",".join(cpp_array)))


async def setup_device_core_(var, config):
    cg.add(var.set_address(config[CONF_MAC_ADDRESS].as_hex))
    if CONF_BINDKEY in config:
        cg.add(var.set_bindkey(as_bindkey(config[CONF_BINDKEY])))


async def new_device(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await esp32_ble_tracker.register_ble_device(var, config)

    await setup_device_core_(var, config)
    if CONF_BATTERY_LEVEL in config:
        sens = await sensor.new_sensor(config[CONF_BATTERY_LEVEL])
        cg.add(var.set_battery_level(sens))
    if CONF_RSSI in config:
        sens = await sensor.new_sensor(config[CONF_RSSI])
        cg.add(var.set_rssi(sens))

    return var


async def new_binary_sensor_device(config):
    var = await new_device(config)
    await binary_sensor.register_binary_sensor(var, config)
    return var


async def new_sensor_device(config):
    var = await new_device(config)
    await sensor.register_sensor(var, config)
    return var


async def to_code(config):
    # var = cg.new_Pvariable(config[CONF_ID])
    # await esp32_ble_tracker.register_ble_device(var, config)
    parent = await cg.get_variable(config[esp32_ble_tracker.CONF_ESP32_BLE_ID])
    for conf in config.get(CONF_ON_MIOT_ADVERTISE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], parent)
        await setup_device_core_(trigger, conf)
        if CONF_PRODUCT_ID in conf:
            cg.add(trigger.set_product_id(conf[CONF_PRODUCT_ID]))
        if conf.get(CONF_DEBUG, False):
            cg.add(trigger.set_debug(True))
        await automation.build_automation(trigger, [(BLEObjectConstRef, "x")], conf)
