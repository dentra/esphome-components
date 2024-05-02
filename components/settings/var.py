from typing import Any

from .. import cgp
from . import const, cpp_utils

# variable types
VT_INT8 = "int8"
VT_UINT8 = "uint8"
VT_INT16 = "int16"
VT_UINT16 = "uint16"
VT_INT32 = "int32"
VT_UINT32 = "uint32"
VT_INT64 = "int64"
VT_UINT64 = "uint64"
VT_FLOAT = "float"
VT_DOUBLE = "double"
VT_BOOL = "bool"
VT_STR = "str"
VT_PASSWORD = "password"
VT_MAC = "mac"
VT_PIN = "pin"
VT_TIMEOUT = "timeout"

VT_STR_TYPES = [VT_STR, VT_PASSWORD]


class Var:
    __slots__ = ("_val", "id")

    def __init__(self, id: str, val: dict[str, Any]) -> None:
        self.id = id
        self._val = val

    def join(self, var: "Var") -> None:
        val = var._val.copy()
        val.update(self._val)
        self._val = val

    def _get(self, key: str, default: Any = None) -> Any | None:
        return self._val.get(key, default)

    def _intl(self, prop: str) -> str | None:
        if prop not in self._val:
            return None
        v = self._get(prop)
        if v is None:
            return None
        v = str(v).strip()
        if len(v) == 0:
            return None
        return v

    @property
    def ifdef(self) -> list[str]:
        return self._get(const.CONF_VAR_IFDEF, [])

    @property
    def section(self) -> str:
        return self._intl(const.CONF_VAR_SECTION)

    @property
    def name(self) -> str:
        name = self._intl(const.CONF_VAR_NAME)
        if not name:
            name = self.id
            section = self.section
            if section:
                if name.lower().startswith(self.section.lower()):
                    name = name[len(self.section) :]
                if name.startswith("_"):
                    name = name[1:]
            name = " ".join(name.split("_")).title()
        return name

    @property
    def help(self) -> str:
        return self._intl(const.CONF_VAR_HELP)

    @property
    def type(self) -> str:
        return self._val[const.CONF_VAR_TYPE]

    @property
    def min(self) -> float | None:
        default = None
        if self.type in [VT_UINT8, VT_UINT16, VT_UINT32, VT_UINT64]:
            default = 0
        elif self.type in VT_STR_TYPES:
            default = 0
        elif self.type == VT_PIN:
            default = cgp.gpio_num_t.GPIO_NUM_NC
        return self._get(const.CONF_VAR_MIN, default)

    @property
    def max(self) -> float | None:
        default = None
        if self.type in VT_STR_TYPES:
            default = const.DEFAULT_STR_SIZE
        elif self.type == VT_INT8:
            default = 0x7F
        elif self.type == VT_UINT8:
            default = 0xFF
        elif self.type == VT_INT16:
            default = 0x7FFF
        elif self.type == VT_UINT16:
            default = 0xFFFF
        elif self.type == VT_INT32:
            default = 0x7FFFFFFF
        elif self.type == VT_UINT32:
            default = 0xFFFFFFFF
        elif self.type == VT_INT64:
            default = 0x7FFFFFFFFFFFFFFF
        elif self.type == VT_UINT64:
            default = 0xFFFFFFFFFFFFFFFF
        elif self.type == VT_PIN:
            default = cgp.gpio_num_t.GPIO_NUM_MAX
        return self._get(const.CONF_VAR_MAX, default)

    @property
    def accuracy_decimals(self) -> int:
        return int(self._get(const.CONF_VAR_ACCURACY_DECIMALS, 0))

    @property
    def component(self) -> str | None:
        return self._get(const.CONF_VAR_COMP)

    @property
    def component_idx(self) -> int:
        return self._get(const.CONF_VAR_COMP_IDX) or 0

    @property
    def component_key(self) -> str | None:
        return self._get(const.CONF_VAR_COMP_KEY)

    @property
    def getter(self):
        return self._get(const.CONF_VAR_GETTER, self._get(const.CONF_VAR_VALUE))

    @property
    def setter(self):
        return self._get(const.CONF_VAR_SETTER)

    @property
    def key(self):
        return f"{cpp_utils.fnv1_hash(self.id):08x}"


def var_list(vars: dict[str, dict[str, Any]]) -> list[Var]:
    return [Var(key, val) for key, val in vars.items()]
