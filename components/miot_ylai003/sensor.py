import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import miot
from esphome.const import (
    CONF_ON_CLICK,
    CONF_ON_DOUBLE_CLICK,
    CONF_TRIGGER_ID,
)

CONF_BUTTON = "button"
CONF_ON_LONG_PRESS = "on_long_press"

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot"]

miot_ylai003_ns = cg.esphome_ns.namespace("miot_ylai003")
MiotYLAI003 = miot_ylai003_ns.class_("MiotYLAI003", miot.MiotComponent)
MiotYLAI003Trigger = miot_ylai003_ns.class_(
    "MiotYLAI003Trigger", automation.Trigger.template(), miot.MiotListener
)
ButtonEventType = miot.miot_ns.namespace("ButtonEvent").enum("Type")

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(MiotYLAI003),
        cv.Optional(CONF_ON_CLICK): automation.validate_automation(
            cv.Schema(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotYLAI003Trigger),
                }
            ),
        ),
        cv.Optional(CONF_ON_DOUBLE_CLICK): automation.validate_automation(
            cv.Schema(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotYLAI003Trigger),
                }
            ),
        ),
        cv.Optional(CONF_ON_LONG_PRESS): automation.validate_automation(
            cv.Schema(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotYLAI003Trigger),
                }
            ),
        ),
    },
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def new_trigger_(config, field, button_type):
    for conf in config.get(field, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], button_type)
        await miot.register_miot_device(trigger, config)
        await miot.setup_device_core_(trigger, config)
        await automation.build_automation(trigger, [], conf)


async def to_code(config):
    await miot.new_device(config)
    await new_trigger_(config, CONF_ON_CLICK, ButtonEventType.CLICK)
    await new_trigger_(config, CONF_ON_DOUBLE_CLICK, ButtonEventType.DOUBLE_CLICK)
    await new_trigger_(config, CONF_ON_LONG_PRESS, ButtonEventType.LONG_PRESS)
