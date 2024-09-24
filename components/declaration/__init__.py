import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import cpp_generator as cpp
from esphome.const import CONF_INTERNAL, CONF_LAMBDA
from esphome.core import Lambda

CONF_NAMESPACE = "namespace"


DECLARATION_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_NAMESPACE): cv.ensure_list(cv.valid_name),
        cv.Optional(CONF_INTERNAL): cv.boolean,
        cv.Required(CONF_LAMBDA): cv.lambda_,
    }
)


def _validate_declaration(conf):
    if not isinstance(conf, dict):
        conf = {CONF_LAMBDA: conf}
    if not isinstance(conf[CONF_LAMBDA], Lambda):
        conf[CONF_LAMBDA] = cv.lambda_(conf[CONF_LAMBDA])
    return conf


CONFIG_SCHEMA = cv.ensure_list(_validate_declaration)


async def to_code(config):
    for conf in config:
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
