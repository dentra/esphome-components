# Dummy integration to allow relying on ETL
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import CORE, coroutine_with_priority

CODEOWNERS = ["@dentra"]

CONFIG_SCHEMA = cv.All(
    cv.Schema({}),
)


@coroutine_with_priority(200.0)
async def to_code(config):
    cg.add_library(
        "Embedded Template Library", None, "https://github.com/ETLCPP/etl@20.35.0"
    )
    # cg.add_library("etlcpp/Embedded Template Library", "20.32.1")
    cg.add_build_flag("-D PROFILE_GCC_GENERIC")
