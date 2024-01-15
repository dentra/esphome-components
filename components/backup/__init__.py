import gzip
import logging
from copy import deepcopy
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.config import strip_default_ids
from esphome.cpp_generator import ArrayInitializer
from esphome.yaml_util import dump
from esphome.core import CORE, coroutine_with_priority, ID
from esphome.components import web_server_base
from esphome.components.web_server_base import CONF_WEB_SERVER_BASE_ID
from esphome.const import (
    CONF_AUTH,
    CONF_ID,
    CONF_PASSWORD,
    CONF_USERNAME,
)

_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["web_server_base"]

CONF_BACKUP = "backup"

backup_ns = cg.esphome_ns.namespace("backup")
Backup = backup_ns.class_("Backup", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Backup),
        cv.GenerateID(CONF_WEB_SERVER_BASE_ID): cv.use_id(
            web_server_base.WebServerBase
        ),
        cv.Optional(CONF_AUTH): cv.Schema(
            {
                cv.Required(CONF_USERNAME): cv.string_strict,
                cv.Required(CONF_PASSWORD): cv.string_strict,
            }
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


def _dump_config():
    cfg = deepcopy(CORE.config)
    cfg = strip_default_ids(cfg)
    return dump(cfg, True)

@coroutine_with_priority(40.0)
async def to_code(config):
    paren = await cg.get_variable(config[CONF_WEB_SERVER_BASE_ID])

    var = cg.new_Pvariable(config[CONF_ID], paren)
    await cg.register_component(var, config)

    if CONF_AUTH in config:
        username = config[CONF_AUTH][CONF_USERNAME]
        if username:
            cg.add(var.set_username(username))
            password = config[CONF_AUTH][CONF_PASSWORD]
            if password:
                cg.add(var.set_password(password))

    tx_config = _dump_config()
    print(tx_config)
    gz_config = gzip.compress(tx_config.encode("utf-8"))
    arr_size = len(gz_config)
    arr_data = ', '.join(f"0x{x:02x}" for x in gz_config)

    cg.add_global(cg.RawExpression(f"const uint8_t ESPHOME_BACKUP_DATA[{arr_size}] PROGMEM = {{{arr_data}}}"))
    cg.add_global(cg.RawExpression(f"const size_t ESPHOME_BACKUP_SIZE = {arr_size}"))

    _LOGGER.info("Backup config will take: %u bytes", arr_size)
