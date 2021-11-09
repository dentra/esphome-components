import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import miot
from esphome.const import (
    CONF_TRIGGER_ID,
)

CONF_ON_SHORT_PRESS = "on_short_press"
CONF_ON_LONG_PRESS = "on_long_press"
CONF_ON_ROTATE_LEFT = "on_rotate_left"
CONF_ON_ROTATE_RIGHT = "on_rotate_right"
CONF_ON_ROTATE_LEFT_PRESSED = "on_rotate_left_pressed"
CONF_ON_ROTATE_RIGHT_PRESSED = "on_rotate_right_pressed"

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot", "sensor"]

miot_ylkg0xyl_ns = cg.esphome_ns.namespace("miot_ylkg0xyl")
MiotYLKG0XYL = miot_ylkg0xyl_ns.class_("MiotYLKG0XYL", miot.MiotComponent)
MiotYLKG0XYLTrigger = miot_ylkg0xyl_ns.class_(
    "MiotYLKG0XYLTrigger", automation.Trigger.template(), miot.MiotListener
)
MiotYLKG0XYLEvent = miot_ylkg0xyl_ns.enum("MiotYLKG0XYLEvent")

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(MiotYLKG0XYL),
        cv.Optional(CONF_ON_SHORT_PRESS): automation.validate_automation(
            cv.Schema(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotYLKG0XYLTrigger)}
            ),
        ),
        cv.Optional(CONF_ON_LONG_PRESS): automation.validate_automation(
            cv.Schema(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotYLKG0XYLTrigger)}
            ),
        ),
        cv.Optional(CONF_ON_ROTATE_LEFT): automation.validate_automation(
            cv.Schema(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotYLKG0XYLTrigger)}
            ),
        ),
        cv.Optional(CONF_ON_ROTATE_RIGHT): automation.validate_automation(
            cv.Schema(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotYLKG0XYLTrigger)}
            ),
        ),
        cv.Optional(CONF_ON_ROTATE_LEFT_PRESSED): automation.validate_automation(
            cv.Schema(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotYLKG0XYLTrigger)}
            ),
        ),
        cv.Optional(CONF_ON_ROTATE_RIGHT_PRESSED): automation.validate_automation(
            cv.Schema(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotYLKG0XYLTrigger)}
            ),
        ),
    },
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def configure_event_trigger_(config, param, enum):
    for conf in config.get(param, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], enum)
        await miot.register_miot_device(trigger, config)
        await miot.setup_device_core_(trigger, config)
        await automation.build_automation(trigger, [(cg.uint8, "x")], conf)


async def to_code(config):
    await configure_event_trigger_(
        config, CONF_ON_SHORT_PRESS, MiotYLKG0XYLEvent.ON_SHORT_PRESS
    )
    await configure_event_trigger_(
        config, CONF_ON_LONG_PRESS, MiotYLKG0XYLEvent.ON_LONG_PRESS
    )
    await configure_event_trigger_(
        config, CONF_ON_ROTATE_LEFT, MiotYLKG0XYLEvent.ON_ROTATE_LEFT
    )
    await configure_event_trigger_(
        config, CONF_ON_ROTATE_RIGHT, MiotYLKG0XYLEvent.ON_ROTATE_RIGHT
    )
    await configure_event_trigger_(
        config, CONF_ON_ROTATE_LEFT_PRESSED, MiotYLKG0XYLEvent.ON_ROTATE_LEFT_PRESSED
    )
    await configure_event_trigger_(
        config, CONF_ON_ROTATE_RIGHT_PRESSED, MiotYLKG0XYLEvent.ON_ROTATE_RIGHT_PRESSED
    )
