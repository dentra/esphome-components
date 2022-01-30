from esphome.cpp_types import Component
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    miot_client,
    sensor,
    text_sensor,
    switch,
    binary_sensor,
    select,
    ble_client,
    esp32_ble_tracker,
    time,
)
from esphome.const import (
    CONF_DEVICE_CLASS,
    CONF_ENTITY_CATEGORY,
    CONF_ICON,
    CONF_ID,
    CONF_TIME_ID,
    CONF_VERSION,
    CONF_POWER,
    CONF_MODE,
    CONF_BATTERY_LEVEL,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_BATTERY_CHARGING,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_PROBLEM,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    UNIT_PERCENT,
    STATE_CLASS_MEASUREMENT,
)

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot_client", "text_sensor", "switch", "sensor", "select"]
DEPENDENCIES = ["time"]

miot_cwbs01_ns = cg.esphome_ns.namespace("miot_cwbs01")
MiotCWBS01 = miot_cwbs01_ns.class_(
    "MiotCWBS01",
    cg.PollingComponent,
    binary_sensor.BinarySensor,
    miot_client.MiotClient,
)

MiotCWBS01PowerSwitch = miot_cwbs01_ns.class_("MiotCWBS01PowerSwitch", switch.Switch)
MiotCWBS01CycleSwitch = miot_cwbs01_ns.class_("MiotCWBS01CycleSwitch", switch.Switch)

MiotCWBS01ModeSelect = miot_cwbs01_ns.class_("MiotCWBS01ModeSelect", select.Select)
MiotCWBS01SceneSelect = miot_cwbs01_ns.class_("MiotCWBS01SceneSelect", select.Select)

CONF_SCENE = "scene"
CONF_CYCLE = "cycle"
CONF_CHARGING = "charging"
CONF_ERROR = "error"

OPTIONS_MODE = [
    # starting from 0
    "None",
    "Keep - Freshing",
    "Anion",
    "Deodorization",
    "Purification",
    "Deep Purification",
]

OPTIONS_SCENE = [
    # starting from 2
    "Mini refrigirator (smaller than 150L)",
    "Middle refrigirator (150L - 300L)",
    "Big refrigirator",
    "5 - seater car",
    "7 - seater car",
    "Pet House",
    "Toilet / Bathroom",
    "Cabinet (shoe cabinet, wardrobe, cabinet)",
]

CONFIG_SCHEMA = (
    binary_sensor.BINARY_SENSOR_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(MiotCWBS01),
            cv.Optional(
                CONF_DEVICE_CLASS, default=DEVICE_CLASS_POWER
            ): binary_sensor.device_class,
            cv.Optional(miot_client.CONF_MIOT_CLIENT_DEBUG): cv.boolean,
            cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
            cv.Optional(CONF_VERSION): text_sensor.TEXT_SENSOR_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
                    cv.Optional(CONF_ICON, default="mdi:git"): cv.icon,
                    cv.Optional(
                        CONF_ENTITY_CATEGORY, default=ENTITY_CATEGORY_DIAGNOSTIC
                    ): cv.entity_category,
                }
            ),
            cv.Optional(CONF_POWER): switch.SWITCH_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(MiotCWBS01PowerSwitch),
                }
            ),
            cv.Optional(CONF_MODE): select.SELECT_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(MiotCWBS01ModeSelect),
                }
            ),
            cv.Optional(CONF_CYCLE): switch.SWITCH_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(MiotCWBS01CycleSwitch),
                }
            ),
            cv.Optional(CONF_SCENE): select.SELECT_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(MiotCWBS01SceneSelect),
                }
            ),
            cv.Optional(CONF_CHARGING): binary_sensor.BINARY_SENSOR_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(binary_sensor.BinarySensor),
                    cv.Optional(
                        CONF_DEVICE_CLASS, default=DEVICE_CLASS_BATTERY_CHARGING
                    ): binary_sensor.device_class,
                    cv.Optional(
                        CONF_ENTITY_CATEGORY, default=ENTITY_CATEGORY_DIAGNOSTIC
                    ): cv.entity_category,
                }
            ),
            cv.Optional(CONF_ERROR): binary_sensor.BINARY_SENSOR_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(binary_sensor.BinarySensor),
                    cv.Optional(
                        CONF_DEVICE_CLASS, default=DEVICE_CLASS_PROBLEM
                    ): binary_sensor.device_class,
                    cv.Optional(
                        CONF_ENTITY_CATEGORY, default=ENTITY_CATEGORY_DIAGNOSTIC
                    ): cv.entity_category,
                }
            ),
            cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            # schedule
        }
    )
    .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA)
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(miot_client.standard_auth_schema())
    .extend(cv.polling_component_schema("1h"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # await miot.register_miot_device(var, config)
    await binary_sensor.register_binary_sensor(var, config)
    await ble_client.register_ble_node(var, config)

    auth = await miot_client.register_standard_auth_client(config)
    cg.add(auth.add_auth_listener(var))
    cg.add(var.set_auth_client(auth))

    tvar = await cg.get_variable(config[CONF_TIME_ID])
    cg.add(var.set_time(tvar))

    if config.get(miot_client.CONF_MIOT_CLIENT_DEBUG, False):
        cg.add(var.set_debug(True))

    if CONF_VERSION in config:
        conf = config[CONF_VERSION]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await text_sensor.register_text_sensor(sens, conf)
        cg.add(var.set_version(sens))

    if CONF_POWER in config:
        conf = config[CONF_POWER]
        sens = cg.new_Pvariable(conf[CONF_ID], var)
        await switch.register_switch(sens, conf)
        cg.add(var.set_power(sens))

    if CONF_MODE in config:
        conf = config[CONF_MODE]
        sens = cg.new_Pvariable(conf[CONF_ID], var)
        await select.register_select(sens, conf, options=OPTIONS_MODE)
        cg.add(var.set_mode(sens))

    if CONF_CYCLE in config:
        conf = config[CONF_CYCLE]
        sens = cg.new_Pvariable(conf[CONF_ID], var)
        await switch.register_switch(sens, conf)
        cg.add(var.set_cycle(sens))

    if CONF_SCENE in config:
        conf = config[CONF_SCENE]
        sens = cg.new_Pvariable(conf[CONF_ID], var)
        await select.register_select(sens, conf, options=OPTIONS_SCENE)
        cg.add(var.set_scene(sens))

    if CONF_CHARGING in config:
        conf = config[CONF_CHARGING]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_charging(sens))

    if CONF_ERROR in config:
        conf = config[CONF_ERROR]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_error(sens))

    if CONF_BATTERY_LEVEL in config:
        conf = config[CONF_BATTERY_LEVEL]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_battery_level(sens))
