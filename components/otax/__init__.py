import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome.core import CORE, ID
from esphome.components import ota, switch
from esphome.const import (
    CONF_OTA,
    CONF_ID,
    ENTITY_CATEGORY_DIAGNOSTIC,
    DEVICE_CLASS_AQI,
)

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["switch"]
DEPENDENCIES = ["ota"]

otax_ns = cg.esphome_ns.namespace("otax")
Otax = otax_ns.class_("Otax", cg.Component)
OtaxSwitch = Otax.class_("Enabled", switch.Switch, cg.Component)

CONF_ENABLED = "enabled"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ENABLED): switch.switch_schema(
            OtaxSwitch,
            icon="mdi:cloud-upload-outline",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            block_inverted=True,
        ),
    }
)


def _final_validate(config):
    ota_conf = fv.full_config.get()[CONF_OTA]
    ota_conf[CONF_ID].type = Otax


FINAL_VALIDATE_SCHEMA = _final_validate


async def to_code(config):
    var = await cg.get_variable(CORE.config[CONF_OTA][CONF_ID])

    enabled = await switch.new_switch(config[CONF_ENABLED])

    cg.add(var.set_enabled(enabled))
