import esphome.config_validation as cv
from esphome.components import esp32
from esphome.core import CORE

from .const import (
    CONF_PARTITION_FLAGS,
    CONF_PARTITION_NAME,
    CONF_PARTITION_OFFSET,
    CONF_PARTITION_SIZE,
    CONF_PARTITION_SUBTYPE,
    CONF_PARTITION_TYPE,
    PARTITION_APP,
)
from .defaults import (
    ARDUINO_DEFAULT_SIZES,
    ARDUINO_PARTITION_ORDER,
    ESP_IDF_DEFAULT_SIZES,
    ESP_IDF_PARTITION_ORDER,
)
from .types import PartitionSubTypes, PartitionTypes


class PartitionEntry:
    tid: int = -1
    sid: int = -1
    name: str | None = None
    size: int = 0
    offset: int | None = None
    flags: str = ""
    _label: str | None = None

    def __init__(self, value, name: str = None) -> None:
        src_value = value

        if isinstance(value, PartitionSubTypes):
            self.tid = value.type_id
            self.sid = value.id
            self.name = name if name else value.label
            return

        if isinstance(value, PartitionTypes):
            self.tid = value.id
            self.sid = -1
            self.name = name
            return

        if isinstance(value, dict):
            self.size = value.get(CONF_PARTITION_SIZE, None) or 0
            self.flags = value.get(CONF_PARTITION_FLAGS, None) or ""
            self.offset = value.get(CONF_PARTITION_OFFSET, None)
            self._label = value.get(CONF_PARTITION_NAME, None)

            typ_id = PartitionEntry._typ_id_by_val(value.get(CONF_PARTITION_TYPE, None))

            sub = value.get(CONF_PARTITION_SUBTYPE, None)

            # first, try to convert string like '0x50' to int
            if isinstance(sub, str):
                try:
                    sub = int(sub, 0)
                except (TypeError, ValueError):
                    pass

            if isinstance(sub, str):
                psub = PartitionEntry._sub_by_str(sub)
                if psub is not None:
                    if typ_id != -1 and psub.type_id != typ_id:
                        raise cv.Invalid(
                            f"Inconsistent type 0x{typ_id:02x} and subtype {psub}"
                        )
                    self.tid = psub.type_id
                    self.sid = psub.id
                    self.name = name if name else psub.label
                    return

            if isinstance(sub, int):
                if typ_id == -1:
                    raise cv.Invalid(f"Unknown partition type for {value}")
                self.tid = typ_id
                self.sid = sub
                self.name = name
                return

            # fallback to search by name
            value = name

        if isinstance(value, str):
            psub = PartitionEntry._sub_by_str(value)
            if psub is not None:
                self.tid = psub.type_id
                self.sid = psub.id
                self.name = name if name else value
                return

        # ValueError
        raise cv.Invalid(
            f"Invalid partition entry: {name if name else value}{f' {src_value}' if src_value else ''}"
        )

    @staticmethod
    def _typ_id_by_val(value) -> int:
        if isinstance(value, int):
            return value

        if value is None:
            return -1

        try:
            return int(value, 0)
        except (TypeError, ValueError):
            pass

        try:
            return PartitionTypes[value.upper()].id
        except (AttributeError, KeyError):
            pass

        return -1

    @staticmethod
    def _typ_by_str(value: str) -> PartitionTypes | None:
        try:
            return PartitionTypes[value.upper()]
        except (AttributeError, KeyError):
            return None

    @staticmethod
    def _sub_by_str(value: str) -> PartitionSubTypes | None:
        # special case for "app"
        if value == PARTITION_APP:
            return PartitionSubTypes.FACTORY

        if value is None:
            return None

        try:
            # serch by enum key
            return PartitionSubTypes[value.upper()]
        except (AttributeError, KeyError):
            pass

        value = value.lower()
        # search by label
        for partition in PartitionSubTypes:
            if partition.label == value:
                return partition

        return None

    @property
    def label(self) -> str:
        return self._label if self._label is not None else self.name

    @property
    def type(self) -> str:
        for typ in PartitionTypes:
            if typ.id == self.tid:
                return typ.type
        return f"0x{self.tid:02x}"

    @property
    def subtype(self) -> str:
        for sub in PartitionSubTypes:
            if sub.type_id == self.tid and sub.id == self.sid:
                return sub.subtype
        return f"0x{self.sid:02x}"

    @property
    def default_size(self) -> int | None:
        if self.is_app_type:
            import esphome.final_validate as fv
            from esphome.components.esp32 import CONF_FLASH_SIZE
            from esphome.components.esp32.const import KEY_ESP32

            flash_size = fv.full_config.get()[KEY_ESP32][CONF_FLASH_SIZE]
            return esp32.APP_PARTITION_SIZES[flash_size]

        for sub in PartitionSubTypes:
            if sub.type_id == self.tid and sub.id == self.sid:
                if CORE.using_esp_idf:
                    return ESP_IDF_DEFAULT_SIZES.get(sub, None)
                if CORE.using_arduino:
                    return ARDUINO_DEFAULT_SIZES.get(
                        sub, ESP_IDF_DEFAULT_SIZES.get(sub, None)
                    )
        return None

    @property
    def is_app_type(self) -> bool:
        return self.tid == PartitionTypes.APP.id

    @property
    def is_valid(self) -> bool:
        if self.tid == -1:
            return False
        if self.sid == -1:
            return False
        if self.name is None:
            return False
        if self.size == 0 and self.default_size is None:
            return False
        return True

    @property
    def order(self):
        orders = {}
        if CORE.using_esp_idf:
            orders = ESP_IDF_PARTITION_ORDER
        elif CORE.using_arduino:
            orders = ARDUINO_PARTITION_ORDER
        for item, value in orders.items():
            if item.id == self.sid and item.type_id == self.tid:
                return value
        return PartitionSubTypes.order_for(self.tid, self.sid)

    def __eq__(self, __value: object) -> bool:
        if isinstance(__value, PartitionEntry):
            return self.tid == __value.tid and self.sid == __value.sid
        if isinstance(__value, PartitionSubTypes):
            return self.tid == __value.type_id and self.sid == __value.id
        return False

    def __hash__(self) -> int:
        return hash(self.name)

    def __str__(self) -> str:
        return f"PartitionEntry[name: {self.name}, tid: {self.tid}, sid: {self.sid}, size: 0x{self.size:x}, offset: {self.offset}, order: {self.order}]"
