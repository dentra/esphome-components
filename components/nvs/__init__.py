import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import PLATFORM_ESP32

nvs_ns = cg.esphome_ns.namespace("nvs_flash")
NVSFlash = nvs_ns.class_("NvsFlash")
NVSFlashConstRef = NVSFlash.operator("ref").operator("const")


CONFIG_SCHEMA = cv.All(
    cv.Schema({}),
    cv.only_on([PLATFORM_ESP32]),
)



async def to_code(config):
    # cg.add_global(nvs_ns.using)
    pass
