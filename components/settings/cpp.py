import json
import logging

import esphome.codegen as cg
import esphome.cpp_generator as cpp
from esphome import core
from esphome.const import CONF_ESPHOME, CONF_ID

from ..codegen import cpp as cgp
from ..nvs import NVSFlashConstRef
from . import var

settings_ns = cg.esphome_ns.namespace("settings")
Settings = settings_ns.class_("Settings", cg.Component)
VariableType = settings_ns.enum("VariableType")
VarInfo = settings_ns.namespace("VarInfo")

_CPP_TYPES = {
    var.VT_INT8: cgp.int8,
    var.VT_UINT8: cg.uint8,
    var.VT_INT16: cg.int16,
    var.VT_UINT16: cg.uint16,
    var.VT_INT32: cg.int32,
    var.VT_UINT32: cg.uint32,
    var.VT_INT64: cg.int64,
    var.VT_UINT64: cg.uint64,
    var.VT_FLOAT: cg.float_,
    var.VT_DOUBLE: cg.double,
    var.VT_BOOL: cg.bool_,
    var.VT_STR: cg.std_string,
    var.VT_PASSWORD: cg.std_string,
    var.VT_PIN: cgp.int8,
    var.VT_MAC: cg.uint64,
    var.VT_TIMEOUT: cg.uint32,
}

_CPP_GETTER_NAMES = {
    var.VT_INT8: "i8",
    var.VT_UINT8: "u8",
    var.VT_INT16: "i16",
    var.VT_UINT16: "u16",
    var.VT_INT32: "i32",
    var.VT_UINT32: "u32",
    var.VT_INT64: "i64",
    var.VT_UINT64: "u64",
    var.VT_FLOAT: "f",
    var.VT_DOUBLE: "d",
    var.VT_BOOL: "b",
    var.VT_STR: "s",
    var.VT_PASSWORD: "s",
    var.VT_MAC: "u64",
    var.VT_PIN: "i8",
    var.VT_TIMEOUT: "u32",
}

_CPP_TYPE_FMT = {
    var.VT_INT8: '" PRIi8 "',
    var.VT_UINT8: '" PRIu8 "',
    var.VT_INT16: '" PRIi16 "',
    var.VT_UINT16: '" PRIu16 "',
    var.VT_INT32: '" PRIi32 "',
    var.VT_UINT32: '" PRIu32 "',
    var.VT_INT64: '" PRIi64 "',
    var.VT_UINT64: '" PRIu64 "',
    var.VT_FLOAT: ".3f",
    var.VT_DOUBLE: ".3f",
    var.VT_BOOL: "s",
    var.VT_STR: "s",
    var.VT_PASSWORD: "s",
    var.VT_MAC: '" PRIX64 "',
    var.VT_PIN: '" PRIi8 "',
    var.VT_TIMEOUT: '" PRIu32 "',
}


def _type(v: var.Var) -> cg.MockObj | None:
    return _CPP_TYPES.get(v.type, None)


def _component(v: var.Var) -> dict | None:
    c = v.component
    if c is None or c not in core.CORE.config:
        return None
    c = core.CORE.config[c]

    if isinstance(c, list):
        idx = v.component_idx
        if idx >= len(c):
            return None
        c = c[idx]

    if c is None:
        return None

    key = v.component_key
    if key:
        if key not in c:
            return None
        c = c[key]

    return c


def _component_obj(v: var.Var) -> cg.MockObj | None:
    if v.component == CONF_ESPHOME:
        return cg.MockObj("App")
    c = _component(v)
    if c is None or CONF_ID not in c:
        return None
    return cgp.MockObjPtr(c[CONF_ID])


async def _getter(v: var.Var) -> cpp.LambdaExpression:
    getter = v.getter

    if isinstance(getter, core.Lambda):
        return await cg.process_lambda(getter, [], capture="", return_type=_type(v))

    if callable(getter):
        c = _component(v)
        if c is None:
            logging.warning("_getter component not found: %s", v.id)
            return None
        getter = getter(c, _component_obj(v))

    if isinstance(getter, core.TimePeriod):
        getter = int(getter.total_milliseconds)
    elif isinstance(getter, core.MACAddress):
        getter = getter.as_hex

    if getter is None:
        logging.warning("_getter is absent: %s", v.id)
        return None

    return cpp.LambdaExpression(
        [f"return {cg.safe_exp(getter)};"], [], "", _type(v), None
    )


def _json_escape_str(v: str, default=cg.RawExpression("nullptr")):
    if v is None:
        return default
    return json.dumps(v)[1:-1]


def _json_escape_float(v: float, default=cg.RawExpression("NAN")):
    if v is None:
        return default
    return v


async def register_vars(settings, vars: list[var.Var]):
    for v in vars:
        getter = await _getter(v)
        if not getter:
            continue
        getter = cgp.IdentExperession(getter)
        getter = cg.StructInitializer("", (_CPP_GETTER_NAMES[v.type], getter))
        getter = cgp.IdentExperession(getter)
        arg = cg.StructInitializer(
            VarInfo,
            ("key", cg.RawExpression(f'"{v.key}" /* fnv1_hash("{v.id}") */')),
            ("type", getattr(VariableType, f"VAR_TYPE_{v.type.upper()}")),
            ("name", _json_escape_str(v.name)),
            ("section", _json_escape_str(v.section)),
            ("help", _json_escape_str(v.help)),
            ("min", _json_escape_float(v.min)),
            ("max", _json_escape_float(v.max)),
            ("accuracy_decimals", v.accuracy_decimals),
            ("getter", getter),
        )
        cg.add(cgp.IfDefStatement(v.ifdef, settings.register_variable(arg)))


def log_expression(key: str, typ: int, x: cg.MockObj):
    fmt = f'"Loaded %s: %{_CPP_TYPE_FMT[typ]}"'
    if fmt.endswith(' ""'):
        fmt = fmt[0:-3]
    if typ == var.VT_STR:
        x = x.c_str()
    elif typ == var.VT_PASSWORD:
        x = "*" * 10
    elif typ == var.VT_BOOL:
        x = cgp.esp_truefalse(x)
    return cgp.esp_logv("main.settings", cg.RawExpression(fmt), key, x)


def log_var_expression(v: var.Var, x: cg.MockObj):
    return log_expression(v.id, v.type, x)


def _wrap_setter(
    v: var.Var, nvs: cg.MockObj, sav: cg.MockObj, obj: cg.MockObj, x: cg.MockObj
) -> cg.Statement:
    asgn = cgp.ConstAutoAssignmentExpression(sav, nvs.get.template(_type(v))(v.key))
    stmt = []
    stmt.append(log_var_expression(v, sav.value()))
    if x is not None:
        val = sav.value()
        if v.type == var.VT_PIN:
            val = cgp.static_cast(cgp.gpio_num_t, val)
        stmt.append(cgp.ConstAutoAssignmentExpression(x, val))
    stmt.append(obj)
    return cgp.IfStatement(f"{asgn}; {sav.has_value()}", stmt)


async def _setter(v: var.Var, nvs: cg.MockObj):
    getter = await _getter(v)
    if not getter:
        return None

    setter = v.setter
    if not setter:
        return None

    x = cg.MockObj("x")
    sav = cg.MockObj("sav")

    if not callable(setter):
        lam: core.Lambda = await cg.process_lambda(setter, [])
        if len(lam.content.strip()) == 0:
            return None
        setter = cg.RawStatement(lam.content)
        setter = _wrap_setter(v, nvs, sav, setter, x)
        return setter

    c = _component(v)
    if c is None:
        logging.warning("_setter component not found: %s", v.id)
        return None

    setter = setter(c, _component_obj(v))
    # or mock object
    if isinstance(setter, cg.MockObj):
        val = sav.value()
        if v.type == var.VT_PIN:
            val = cgp.static_cast(cgp.gpio_num_t, val)
        setter = _wrap_setter(v, nvs, sav, setter(val), None)

    return setter


async def add_on_load(settings, vars: list[var.Var], lambda_: core.Lambda | None):
    nvs = cg.MockObj("nvs")

    ss = []
    for v in vars:
        setter = await _setter(v, nvs)
        if setter:
            ss.append(cgp.IfDefStatement(v.ifdef, setter))

    lv = str(cgp.StatementList(ss))
    if lambda_ is not None and len(str(lambda_.value).strip()) > 0:
        lv += "\n" + str(lambda_.value).strip()

    on_load = await cg.process_lambda(
        core.Lambda(lv), [(NVSFlashConstRef, nvs)], capture=""
    )
    cg.add(settings.load(on_load))
