import logging

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32
from esphome.core import CORE

from .const import *
from .entry import PartitionEntry
from .generator import generate_csv
from .validators import (
    final_validate_config,
    partition_flags,
    partition_name,
    partition_size,
    partition_subtype,
    partition_type,
)

CODEOWNERS = ["@dentra"]

_LOGGER = logging.getLogger(__name__)


def validate_size_shorthand(value):
    if value is None:
        return PARTITION_SCHEMA({})
    # if CORE.using_arduino:
    #     raise cv.Invalid("Shorthand available only for esp-idf")
    if isinstance(value, int):
        return PARTITION_SCHEMA({CONF_PARTITION_SIZE: value})

    raise cv.Invalid("Shorthand only for empty or int type")


PARTITION_SCHEMA = cv.Any(
    cv.Schema(
        {
            cv.Optional(CONF_PARTITION_NAME): partition_name,
            cv.Optional(CONF_PARTITION_TYPE): partition_type(),
            cv.Optional(CONF_PARTITION_SUBTYPE): partition_subtype(),
            cv.Optional(CONF_PARTITION_OFFSET): cv.uint32_t,
            cv.Optional(CONF_PARTITION_SIZE): partition_size,
            cv.Optional(CONF_PARTITION_FLAGS): partition_flags(),
        }
    ),
    validate_size_shorthand,
)

CONFIG_SCHEMA = cv.All(
    cv.Schema({str: PARTITION_SCHEMA}),
)


FINAL_VALIDATE_SCHEMA = final_validate_config


async def to_code(config):
    if not config:
        return

    partitions = [PartitionEntry(item, name) for name, item in config.items()]
    partitions = sorted(partitions, key=lambda x: x.order)

    if CORE.using_esp_idf:
        _LOGGER.info("ESP-IDF partition table:")
        generate_csv(partitions, _LOGGER)
        esp32.add_idf_sdkconfig_option(
            "CONFIG_PARTITION_TABLE_CUSTOM_FILENAME", PARTITIONS_FILENAME
        )

    if CORE.using_arduino:
        _LOGGER.info("Arduino partition table:")
        generate_csv(partitions, _LOGGER)

    cg.add_platformio_option("board_build.partitions", PARTITIONS_FILENAME)
