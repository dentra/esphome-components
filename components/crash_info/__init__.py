import logging
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import CORE
from esphome.cpp_types import Component
from esphome.components import binary_sensor
from esphome.const import (
    DEVICE_CLASS_PROBLEM,
    ENTITY_CATEGORY_DIAGNOSTIC,
    PLATFORM_ESP8266,
    CONF_ID,
)

_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@dentra"]
ESP_PLATFORMS = [PLATFORM_ESP8266]
AUTO_LOAD = ["binary_sensor"]

CONF_MAX_STACK_FRAMES_SIZE = "max_stack_frames_size"
CONF_MIN_STACK_FRAMES_ADDR = "min_stack_frames_addr"
CONF_MAX_STACK_FRAMES_ADDR = "max_stack_frames_addr"
CONF_INDICATOR = "indicator"
CONF_STORE_IN_FLASH = "store_in_flash"

crash_info_ns = cg.esphome_ns.namespace("crash_info")
CrashInfo = crash_info_ns.class_("CrashInfo", Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CrashInfo),
        cv.Optional(CONF_MAX_STACK_FRAMES_SIZE, default=10): cv.All(
            cv.positive_int, cv.int_range(4, 60)
        ),
        cv.Optional(CONF_MIN_STACK_FRAMES_ADDR, default=0x40000000): cv.All(
            cv.positive_int, cv.int_range(1, 0x70000000)
        ),
        cv.Optional(CONF_MAX_STACK_FRAMES_ADDR, default=0x50000000): cv.All(
            cv.positive_int, cv.int_range(1, 0x70000000)
        ),
        cv.Optional(CONF_INDICATOR): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_PROBLEM,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_STORE_IN_FLASH, default=False): cv.boolean,
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_INDICATOR in config:
        conf = config[CONF_INDICATOR]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await binary_sensor.register_binary_sensor(sens, conf)
        cg.add(var.set_indicator(sens))

    cg.add_build_flag(
        f"-DDENTRA_CRASH_INFO_MAX_STACK_FRAMES_SIZE={config[CONF_MAX_STACK_FRAMES_SIZE]}"
    )
    cg.add_build_flag(
        f"-DDENTRA_CRASH_INFO_MIN_STACK_FRAMES_ADDR={config[CONF_MIN_STACK_FRAMES_ADDR]}"
    )
    cg.add_build_flag(
        f"-DDENTRA_CRASH_INFO_MAX_STACK_FRAMES_ADDR={config[CONF_MAX_STACK_FRAMES_ADDR]}"
    )
    cg.add_build_flag(
        f"-DDENTRA_CRASH_INFO_STORE_IN_FLASH={str(config[CONF_STORE_IN_FLASH]).lower()}"
    )

    _LOGGER.info(
        "Crash info will take %u bytes of %s memory%s",
        ((config[CONF_MAX_STACK_FRAMES_SIZE] * 4) + 2)
        + (8 if "time" in CORE.loaded_integrations else 0),
        "FLASH" if config[CONF_STORE_IN_FLASH] else "RTC",
        " (extended with time)" if "time" in CORE.loaded_integrations else "",
    )
