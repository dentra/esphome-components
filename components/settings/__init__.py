import os

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core
from esphome.components import web_server, web_server_base
from esphome.const import CONF_ID, CONF_LAMBDA

from . import const, cpp, presets, var

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["web_server_base", "json", "nvs"]
DEPENDENCIES = ["wifi"]


CONF_SETTINGS_VARIABLES = "settings_variables"
CONF_VARIABLES = "variables"
CONF_PRESETS = "presets"

VAR_TYPE_ALIASES = {
    # string
    "str": var.VT_STR,
    "string": var.VT_STR,
    # char
    "char": var.VT_INT8,
    "int8": var.VT_INT8,
    "int8_t": var.VT_INT8,
    # byte
    "byte": var.VT_UINT8,
    "uint8": var.VT_UINT8,
    "uint8_t": var.VT_UINT8,
    # short
    "short": var.VT_INT16,
    "int16": var.VT_INT16,
    "int16_t": var.VT_INT16,
    # ushort
    "ushort": var.VT_UINT16,
    "uint16": var.VT_UINT16,
    "uint16_t": var.VT_UINT16,
    # integer
    "int": var.VT_INT32,
    "integer": var.VT_INT32,
    "int32": var.VT_INT32,
    "int32_t": var.VT_INT32,
    # unsigned
    "uint": var.VT_UINT32,
    "unsigned": var.VT_UINT32,
    "uint32": var.VT_UINT32,
    "uint32_t": var.VT_UINT32,
    # float
    "float": var.VT_FLOAT,
    "num": var.VT_FLOAT,
    "number": var.VT_FLOAT,
    # double
    "double": var.VT_DOUBLE,
    "real": var.VT_DOUBLE,
    # boolean
    "bool": var.VT_BOOL,
    "boolean": var.VT_BOOL,
    # string password
    "password": var.VT_PASSWORD,
    "pass": var.VT_PASSWORD,
    # mac
    "mac": var.VT_MAC,
    # pin
    "pin": var.VT_PIN,
    # timeout
    "timeout": var.VT_TIMEOUT,
}


def validate_variable(config):
    if const.CONF_VAR_VALUE in config and const.CONF_VAR_GETTER in config:
        raise cv.Invalid(
            f"Only {const.CONF_VAR_VALUE} or {const.CONF_VAR_GETTER} is allowed"
        )

    if const.CONF_VAR_PRESET in config:
        return config

    if not (const.CONF_VAR_VALUE in config or const.CONF_VAR_GETTER in config):
        raise cv.Invalid(
            f"{const.CONF_VAR_VALUE} or {const.CONF_VAR_GETTER} is required"
        )

    if const.CONF_VAR_TYPE not in config:
        if const.CONF_VAR_GETTER in config:
            raise cv.Invalid(f"{const.CONF_VAR_TYPE} is required with getter")

        data = config[const.CONF_VAR_VALUE]
        if isinstance(data, core.Lambda):
            raise cv.Invalid(f"{const.CONF_VAR_TYPE} is required with lambda value")

        if isinstance(data, core.MACAddress):
            config[const.CONF_VAR_TYPE] = var.VT_MAC
        elif isinstance(data, core.TimePeriod):
            config[const.CONF_VAR_TYPE] = var.VT_TIMEOUT
        elif isinstance(data, bool):
            config[const.CONF_VAR_TYPE] = var.VT_BOOL
        elif isinstance(data, int):
            config[const.CONF_VAR_TYPE] = var.VT_INT32
        elif isinstance(data, float):
            config[const.CONF_VAR_TYPE] = var.VT_FLOAT
        elif isinstance(data, str):
            config[const.CONF_VAR_TYPE] = var.VT_STR
        else:
            raise cv.Invalid(f"Unsupported type {type(data)}")
    else:
        config[const.CONF_VAR_TYPE] = VAR_TYPE_ALIASES[config[const.CONF_VAR_TYPE]]

    typ = config[const.CONF_VAR_TYPE]

    if typ in var.VT_STR_TYPES:
        slen = config.get(const.CONF_VAR_MAX, None)
        if not slen:
            config[const.CONF_VAR_MAX] = const.DEFAULT_STR_SIZE
        elif slen > const.MAX_STR_SIZE:
            raise cv.Invalid(f"Maximum string length is {const.MAX_STR_SIZE}")

    if typ == var.VT_TIMEOUT and not isinstance(
        config[const.CONF_VAR_VALUE], core.TimePeriod
    ):
        config[const.CONF_VAR_VALUE] = core.TimePeriod(
            milliseconds=int(config[const.CONF_VAR_VALUE])
        )

    return config


def variable_shorthand(config):
    if not isinstance(config, dict):
        config = {const.CONF_VAR_VALUE: config}
    return config


VARIABLE_DATA = cv.Any(
    cv.mac_address,
    cv.positive_time_period,
    lambda value: value,  # any yaml type
)


VARIABLE_SCHEMA = cv.Any(
    cv.Schema(
        {
            cv.Optional(const.CONF_VAR_VALUE): VARIABLE_DATA,
            cv.Optional(const.CONF_VAR_SETTER): cv.lambda_,
            cv.Optional(const.CONF_VAR_GETTER): cv.lambda_,
            cv.Optional(const.CONF_VAR_TYPE): cv.one_of(*VAR_TYPE_ALIASES),
            cv.Optional(const.CONF_VAR_SECTION): cv.string,
            cv.Optional(const.CONF_VAR_NAME): cv.string,
            cv.Optional(const.CONF_VAR_HELP): cv.string,
            cv.Optional(const.CONF_VAR_MIN): cv.float_,
            cv.Optional(const.CONF_VAR_MAX): cv.float_,
            cv.Optional(const.CONF_VAR_ACCURACY_DECIMALS): cv.All(
                cv.uint8_t, cv.Range(0, 3)
            ),
            cv.Optional(const.CONF_VAR_IFDEF): cv.ensure_list(cv.valid_name),
            cv.Optional(const.CONF_VAR_PRESET): cv.one_of(*presets.PRESETS, lower=True),
        }
    ),
    cv.All(VARIABLE_DATA, variable_shorthand),
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(cpp.Settings),
        cv.GenerateID(web_server_base.CONF_WEB_SERVER_BASE_ID): cv.use_id(
            web_server_base.WebServerBase,
        ),
        cv.Required(CONF_VARIABLES): cv.Schema(
            {
                cv.valid_name: cv.All(
                    VARIABLE_SCHEMA,
                    validate_variable,
                )
            }
        ),
        cv.Optional(CONF_LAMBDA): cv.lambda_,
        cv.Optional(CONF_PRESETS): cv.ensure_list(
            cv.one_of(*presets.PRESETS, lower=True)
        ),
    }
)


# Run with low priority so that all initilization be doing first
@core.coroutine_with_priority(-200.0)
async def to_code(config):
    # for key, val in CORE.config["mqtt"].items():
    #     print("to_code", key, val)
    #     # for k, v in val.items():
    #     #     print("  ", k, v)

    presets.presets_init(config.get(CONF_PRESETS), config[CONF_VARIABLES])

    web = await cg.get_variable(config[web_server_base.CONF_WEB_SERVER_BASE_ID])
    settings = cg.new_Pvariable(config[CONF_ID], web)
    await cg.register_component(settings, config)

    vars = var.var_list(config[CONF_VARIABLES])
    # registering vars first
    await cpp.register_vars(settings, vars)
    # last step, loading settings
    await cpp.add_on_load(settings, vars, config.get(CONF_LAMBDA, None))

    path = f"{os.path.dirname(__file__)}/settings.html"
    with open(file=path, encoding="utf-8") as html_file:
        web_server.add_resource_as_progmem("SETTINGS_HTML", html_file.read())

    if 'web_server' not in core.CORE.config:
        cg.add(settings.set_base_url("/"))
