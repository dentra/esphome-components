import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32, web_server_base
from esphome.components.web_server_base import CONF_WEB_SERVER_BASE_ID
from esphome.const import CONF_ID, __version__ as ESPHOME_VERSION
from esphome.core import CORE

KEY_PARTITIONS = esp32.CONF_PARTITIONS
IS_ESPHOME_2026_11 = cv.Version.parse(ESPHOME_VERSION) >= cv.Version.parse("2026.11.0")

AUTO_LOAD = (
    ["web_server_base"] if IS_ESPHOME_2026_11 else ["web_server_base", KEY_PARTITIONS]
)
DEPENDENCIES = ["wifi"]
CODEOWNERS = ["@dentra"]

CONF_ENABLE_TEST_CRASH = "enable_test_crash"

coredump_ns = cg.esphome_ns.namespace("coredump")
Coredump = coredump_ns.class_("Coredump", cg.Component)


def update_partitions(config):
    if IS_ESPHOME_2026_11:
        return config
    CORE.data[KEY_PARTITIONS] = {"coredump": {"size": 0x10000}}
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Coredump),
            cv.GenerateID(CONF_WEB_SERVER_BASE_ID): cv.use_id(
                web_server_base.WebServerBase
            ),
            cv.Optional(CONF_ENABLE_TEST_CRASH, default=False): cv.boolean,
        }
    ).extend(cv.COMPONENT_SCHEMA),
    cv.only_with_framework("esp-idf"),
    update_partitions,
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_WEB_SERVER_BASE_ID])
    var = cg.new_Pvariable(config[CONF_ID], paren)
    await cg.register_component(var, config)
    if IS_ESPHOME_2026_11:
        esp32.include_builtin_idf_component("espcoredump")
        esp32.add_partition("coredump", "data", "coredump", 0x10000)
    esp32.add_idf_sdkconfig_option("CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH", True)
    esp32.add_idf_sdkconfig_option("CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF", True)
    esp32.add_idf_sdkconfig_option("CONFIG_ESP_COREDUMP_CHECKSUM_CRC32", True)
    esp32.add_idf_sdkconfig_option("CONFIG_ESP_COREDUMP_CHECK_BOOT", True)
    if config[CONF_ENABLE_TEST_CRASH]:
        cg.add_define("USE_COREDUMP_ENABLE_TEST_CRASH")
