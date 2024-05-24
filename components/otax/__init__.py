import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome.components import switch
from esphome.const import (
    CONF_ESPHOME,
    CONF_ID,
    CONF_OTA,
    CONF_PLATFORM,
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from esphome.const import __version__ as ESPHOME_VERSION
from esphome.core import CORE, ID

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["switch"]
DEPENDENCIES = [
    "ota"
    if cv.Version.parse(ESPHOME_VERSION) < cv.Version.parse("2024.6.0")
    else "ota.esphome"
]

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


def _get_ota_component(full_config) -> ID:
    ota_conf = full_config[CONF_OTA]

    if cv.Version.parse(ESPHOME_VERSION) < cv.Version.parse("2024.6.0"):
        return ota_conf[CONF_ID]

    for conf in ota_conf:
        if conf[CONF_PLATFORM] == CONF_ESPHOME:
            return conf[CONF_ID]

    return None


def _final_validate(config):
    ota_comp = _get_ota_component(fv.full_config.get())
    if not ota_comp:
        raise cv.Invalid(f"No {CONF_ESPHOME} ota platform found")
    ota_comp.type = Otax


FINAL_VALIDATE_SCHEMA = _final_validate


async def to_code(config):
    var = await cg.get_variable(_get_ota_component(CORE.config))

    enabled = await switch.new_switch(config[CONF_ENABLED])

    cg.add(var.set_enabled(enabled))
