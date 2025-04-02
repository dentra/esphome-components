from .. import miot  # pylint: disable=relative-beyond-top-level
from ..miot_th import sensor as miot_th


CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot_th"]
CONFIG_SCHEMA = miot_th.CONFIG_SCHEMA


async def to_code(config):
    """Code generation entry point"""
    miot.deprecated(config, "miot_th")
    await miot_th.to_code(config)
