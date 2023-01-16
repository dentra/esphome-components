import logging
from ..miot_th import sensor as miot_th
from esphome.const import CONF_PLATFORM

_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot_th"] + miot_th.AUTO_LOAD
CONFIG_SCHEMA = miot_th.CONFIG_SCHEMA


async def to_code(config):
    """Code generation entry point"""
    platform = config[CONF_PLATFORM]
    _LOGGER.warning("%s was deprecated, please use miot_th instead.", platform)
    await miot_th.to_code(config)

