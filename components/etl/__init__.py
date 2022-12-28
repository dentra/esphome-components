# Dummy integration to allow relying on ETL
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import coroutine_with_priority

CODEOWNERS = ["@dentra"]

CONFIG_SCHEMA = cv.Schema({})


@coroutine_with_priority(200.0)
async def to_code(config):
    cg.add_library("etlcpp/Embedded Template Library", "20.35.8")
