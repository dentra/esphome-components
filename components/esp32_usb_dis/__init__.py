import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.core import CORE, coroutine_with_priority

from esphome.components.esp32.const import (
    KEY_ESP32,
    KEY_VARIANT,
    VARIANT_ESP32S3,
    VARIANT_ESP32C3,
    VARIANT_ESP32S2,
)

esp32_usb_dis_ns = cg.esphome_ns.namespace("esp32_usb_dis")


def only_on_esp32_variant(variants):
    if not isinstance(variants, list):
        variants = [variants]

    def validator_(obj):
        if CORE.data[KEY_ESP32][KEY_VARIANT] not in variants:
            raise cv.Invalid(f"This feature is only available on {variants}")
        return obj

    return validator_


CONFIG_SCHEMA = cv.All(
    cv.only_on_esp32,
    only_on_esp32_variant([VARIANT_ESP32S3, VARIANT_ESP32C3, VARIANT_ESP32S2]),
    # cv.only_with_esp_idf,
)


@coroutine_with_priority(999.0)
async def to_code(config):
    cg.add(esp32_usb_dis_ns.pre_setup())
