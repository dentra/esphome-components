import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import (
    CONF_ON_CLICK,
    CONF_TRIGGER_ID,
)
from .. import miot  # pylint: disable=relative-beyond-top-level

CONF_BUTTON = "button"
CONF_ON_LONG_PRESS = "on_long_press"

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

miot_ylyk01yl_ns = cg.esphome_ns.namespace("miot_ylyk01yl")
# MiotYLYK01YL = miot_ylyk01yl_ns.class_("MiotYLYK01YL", miot.MiotComponent)
MiotYLYK01YLTrigger = miot_ylyk01yl_ns.class_(
    "MiotYLYK01YLTrigger", automation.Trigger.template(), miot.MiotListener
)
ButtonEventType = miot.miot_ns.namespace("ButtonEvent").enum("Type")

BUTTON_NAMES = {
    "on": 0,
    "off": 1,
    "dimmable": 2,
    "sun": 2,
    "moon": 2,
    "plus": 3,
    "+": 3,
    "moonlight": 4,
    "m": 4,
    "minus": 5,
    "-": 5,
}

CONFIG_SCHEMA = cv.Schema(
    {
        # cv.GenerateID(): cv.declare_id(MiotYLYK01YL),
        cv.Optional(CONF_ON_CLICK): automation.validate_automation(
            cv.Schema(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotYLYK01YLTrigger),
                    cv.Required(CONF_BUTTON): cv.one_of(*BUTTON_NAMES, lower=True),
                }
            ),
        ),
        cv.Optional(CONF_ON_LONG_PRESS): automation.validate_automation(
            cv.Schema(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotYLYK01YLTrigger),
                    cv.Required(CONF_BUTTON): cv.one_of(*BUTTON_NAMES, lower=True),
                }
            ),
        ),
    },
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def configure_event_trigger_(config, param, enum):
    for conf in config.get(param, []):
        trigger = cg.new_Pvariable(
            conf[CONF_TRIGGER_ID], enum, BUTTON_NAMES[conf[CONF_BUTTON]]
        )
        await miot.register_miot_device(trigger, config)
        await miot.setup_device_core_(trigger, config)
        await automation.build_automation(trigger, [], conf)


async def to_code(config):
    """Code generation entry point"""
    await configure_event_trigger_(
        config,
        CONF_ON_CLICK,
        ButtonEventType.BUTTON_CLICK,
    )
    await configure_event_trigger_(
        config,
        CONF_ON_LONG_PRESS,
        ButtonEventType.BUTTON_LONG_PRESS,
    )
