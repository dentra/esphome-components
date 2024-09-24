import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import CONF_PRIORITY, CONF_TRIGGER_ID

OnetimeInitTrigger = cg.esphome_ns.namespace("onetime_init").class_(
    "OnetimeInitTrigger", cg.Component, automation.Trigger.template()
)


CONFIG_SCHEMA = automation.validate_automation(
    {
        cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(OnetimeInitTrigger),
        cv.Optional(CONF_PRIORITY, default=600.0): cv.float_,
    }
)


async def to_code(config):
    for conf in config:
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], conf.get(CONF_PRIORITY))
        await cg.register_component(trigger, conf)
        await automation.build_automation(trigger, [], conf)
