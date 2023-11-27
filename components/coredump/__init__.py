import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32, partitions, web_server_base
from esphome.components.web_server_base import CONF_WEB_SERVER_BASE_ID
from esphome.const import CONF_ID
from esphome.core import CORE, coroutine_with_priority

KEY_PARTITIONS = partitions.DOMAIN

AUTO_LOAD = ["web_server_base", KEY_PARTITIONS]
DEPENDENCIES = ["wifi"]
CODEOWNERS = ["@dentra"]

coredump_ns = cg.esphome_ns.namespace("coredump")
Coredump = coredump_ns.class_("Coredump", cg.Component)


def update_partitions(config):
    CORE.data[KEY_PARTITIONS] = {"coredump": {"size": 0x10000}}
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Coredump),
            cv.GenerateID(CONF_WEB_SERVER_BASE_ID): cv.use_id(
                web_server_base.WebServerBase
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    cv.only_with_esp_idf,
    update_partitions,
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_WEB_SERVER_BASE_ID])
    var = cg.new_Pvariable(config[CONF_ID], paren)
    await cg.register_component(var, config)
    esp32.add_idf_sdkconfig_option("CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH", True)
    esp32.add_idf_sdkconfig_option("CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF", True)
    esp32.add_idf_sdkconfig_option("CONFIG_ESP_COREDUMP_CHECKSUM_CRC32", True)
    esp32.add_idf_sdkconfig_option("CONFIG_ESP_COREDUMP_CHECK_BOOT", True)
