import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core
from esphome import cpp_generator as cpp

CONFIG_SCHEMA = cv.lambda_


@core.coroutine_with_priority(-300.0)
async def to_code(config):
    lambda_: cpp.LambdaExpression = await cg.process_lambda(config, [])
    content = lambda_.content
    if lambda_.source:
        content = f"{lambda_.source.as_line_directive}\n{content}"
    content = cpp.indent_all_but_first_and_last(f"{{\n{content}\n}}")
    cg.add(cg.RawStatement(content))
