import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import miot
from esphome.const import (
    CONF_TRIGGER_ID,
)

CONF_BUTTON = "button"
CONF_ON_TWIST = "on_twist"

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot", "sensor"]

miot_xmmfo1jqd_ns = cg.esphome_ns.namespace("miot_xmmfo1jqd")
MiotXMMFO1JQD = miot_xmmfo1jqd_ns.class_("MiotXMMFO1JQD", miot.MiotComponent)
MiotXMMFO1JQDTrigger = miot_xmmfo1jqd_ns.class_(
    "MiotXMMFO1JQDTrigger", automation.Trigger.template(), miot.MiotListener
)
ButtonEventType = miot.miot_ns.namespace("ButtonEvent").enum("Type")

BUTTON_NAMES = {
    "clockwise": 0,
    "counterclockwise": 1,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(MiotXMMFO1JQD),
        cv.Optional(CONF_ON_TWIST): automation.validate_automation(
            cv.Schema(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MiotXMMFO1JQDTrigger),
                    cv.Required(CONF_BUTTON): cv.one_of(*BUTTON_NAMES, lower=True),
                }
            ),
        ),
    },
).extend(miot.MIOT_BLE_DEVICE_SCHEMA)


async def to_code(config):
    for conf in config.get(CONF_ON_TWIST, []):
        trigger = cg.new_Pvariable(
            conf[CONF_TRIGGER_ID],
            ButtonEventType.BUTTON_CLICK,
            BUTTON_NAMES[conf[CONF_BUTTON]],
        )
        await miot.register_miot_device(trigger, config)
        await miot.setup_device_core_(trigger, config)
        await automation.build_automation(trigger, [], conf)
