import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import cpp_generator as cpp
from esphome.const import CONF_ID, CONF_INTERNAL, CONF_LAMBDA

CONF_NAMESPACE = "namespace"
CONF_INCLUDE = "include"

DECLARATION_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_ID): cv.declare_id(cg.uint8),
        cv.Optional(CONF_NAMESPACE): cv.ensure_list(cv.valid_name),
        cv.Optional(CONF_INTERNAL): cv.boolean,
        cv.Optional(CONF_LAMBDA): cv.lambda_,
        cv.Optional(CONF_INCLUDE): cv.ensure_list(cv.string),
    }
).add_extra(
    cv.has_at_least_one_key(
        CONF_LAMBDA,
        CONF_INCLUDE,
    )
)

CONFIG_SCHEMA = cv.ensure_list(DECLARATION_SCHEMA)


async def to_code(config):
    includes = set()
    # process includes first
    for conf in config:
        for inc in conf.get(CONF_INCLUDE, []):
            includes.add(inc)
    for inc in includes:
        cg.add_global(cg.RawStatement(f'#include "{inc}"'))

    for conf in config:
        if CONF_LAMBDA not in conf:
            continue

        lambda_: cpp.LambdaExpression = await cg.process_lambda(conf[CONF_LAMBDA], [])
        content = lambda_.content

        namespaces = conf.get(CONF_NAMESPACE, [])
        if conf.get(CONF_INTERNAL, True):
            namespaces = ["/* anonymous */", *namespaces]

        ns_prefix = ""
        ns_suffix = ""
        for ns in namespaces:
            ns_prefix += f"namespace {ns} {{\n"
        for ns in reversed(namespaces):
            ns_suffix += f"\n}} // namespace {ns}"

        line = lambda_.source.as_line_directive
        content = f"{line}\n{content}"

        cg.add_global(cg.RawStatement(f"{ns_prefix}{content}{ns_suffix}"))
