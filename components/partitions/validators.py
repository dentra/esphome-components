from enum import Enum, IntEnum, StrEnum

import esphome.config_validation as cv
from esphome.core import CORE

from .const import (
    CONF_PARTITION_FLAGS,
    CONF_PARTITION_NAME,
    CONF_PARTITION_OFFSET,
    CONF_PARTITION_SIZE,
    CONF_PARTITION_SUBTYPE,
    CONF_PARTITION_TYPE,
    DOMAIN,
    PARTITION_APP,
)
from .defaults import ARDUINO_DEFAULT_PARTITIONS, ESP_IDF_DEFAULT_PARTITIONS
from .entry import PartitionEntry
from .types import PartitionFlags, PartitionSubTypes, PartitionTypes


def one_of_enum(value, **kwargs):
    if issubclass(value, StrEnum):
        return cv.one_of(*[i.value for i in value], **kwargs)
    if issubclass(value, IntEnum):
        return cv.one_of(*[str(i.name).lower() for i in value], **kwargs)
    if issubclass(value, Enum):
        return cv.one_of(*[str(i.name).lower() for i in value], **kwargs)
    raise cv.Invalid(f"Unsupported enum type {type(value)}")


# Name field can be any meaningful name. It is not significant to the ESP32.
# The maximum length of names is 16 bytes, including one null terminator.
# Names longer than the maximum length will be truncated.
def partition_name(value):
    value = cv.validate_id_name(value)
    if len(value) > 15:
        raise cv.Invalid(
            f'Invalid partition name "{value}". Maximum length is 15 bytes.'
        )
    return value


def partition_size(value):
    if value is None or value == 0:
        return value
    page_size = 0x1000
    if value <= 0 or value % page_size != 0:
        val = int(value / page_size)
        min_valid = (val + 0) * page_size
        max_valid = (val + 1) * page_size
        raise cv.Invalid(
            f"Invalid partition size 0x{value:X}, change to 0x{min_valid:X} or 0x{max_valid:X}"
        )
    return value


# Partition type field can be specified as app (0x00) or data (0x01).
# Or it can be a number 0-254 (or as hex 0x00-0xFE).
# Types 0x00-0x3F are reserved for ESP-IDF core functions.
# If your app needs to store data in a format not already supported by ESP-IDF,
# then please add a custom partition type value in the range 0x40-0xFE.
def partition_type():
    return cv.Any(
        one_of_enum(PartitionTypes, lower=True),
        cv.int_range(0x00, 0x01),
        cv.int_range(0x40, 0xFE),
    )


# The 8-bit SubType field is specific to a given partition type.
# ESP-IDF currently only specifies the meaning of the subtype field for app
# and data partition types.
# * When type is app, the SubType field can be specified as factory (0x00),
#   ota_0 (0x10) … ota_15 (0x1F) or test (0x20).
# * When type is data, the subtype field can be specified as ota (0x00),
#   phy (0x01), nvs (0x02), nvs_keys (0x04), or a range of other component-specific subtypes.
def partition_subtype():
    return cv.Any(one_of_enum(PartitionSubTypes, lower=True), cv.int_range(0, 254))


def partition_flags():
    return one_of_enum(PartitionFlags, lower=True)


# The partition table length is 0xC00 bytes, as we allow a maximum of 95 entries.
# An MD5 checksum, used for checking the integrity of the partition table at runtime,
# is appended after the table data. Thus, the partition table occupies an entire
# flash sector, which size is 0x1000 (4 KB). As a result, any partition following
# it must be at least located at (default offset) + 0x1000.
def validate_partitions(config: dict):
    if len(config) > 95:
        raise cv.Invalid("Only 95 partition entries allowed")

    app_size = 0

    factory = None
    for name, value in config.items():
        entry = PartitionEntry(value, name)
        if entry == PartitionSubTypes.FACTORY:
            if factory:
                raise cv.Invalid(
                    f"Only one factory partition is allowed: {factory.name}, {entry.name}"
                )
            factory = entry
            app_size = factory.size

    app_found = 0
    app_count = 0

    for name, value in config.items():
        entry = PartitionEntry(value, name)
        name = entry.name
        if name == PARTITION_APP:
            app_found += 1
        elif entry.is_app_type:
            app_count += 1

    if app_found > 1:
        raise cv.Invalid(
            f'Only one special "{PARTITION_APP}" partition must be defined'
        )

    if app_found > 0 and app_count > 0:
        raise cv.Invalid(f"Only {PARTITION_APP} or otaX partition must be defined")

    for name, value in config.items():
        entry = PartitionEntry(value, name)
        if entry.is_app_type and app_size != 0 and app_size != entry.size:
            raise cv.Invalid("App, factory and ota partition sizes must be the same")

    for name, value in config.items():
        size = value.get(CONF_PARTITION_SIZE, None)
        if size is not None and size == 0:
            # skip entry with explicity defined size 0
            continue
        entry = PartitionEntry(value, name)
        if not entry.is_valid:
            raise cv.Invalid(f"Invalid partition {name} attribute")

    return config


def _check_partition_align(entry: PartitionEntry, align: int = 0x1000):
    if entry.size != 0 and (entry.size % align) != 0:
        raise cv.Invalid(f"{entry.name} partition must be aligned to 0x{align:X}")


def apply_app_partitions(config: dict):
    app_partition_align = 0x10000
    # check for one of factrory or ota partitions exists
    for name, value in config.items():
        if name == PARTITION_APP:
            continue
        entry = PartitionEntry(value, name)
        if entry.is_app_type:
            _check_partition_align(entry, app_partition_align)
            return config

    default_partitions = _default_partitions_entries(config)

    # special app partition
    app = config.get(PARTITION_APP, {})
    papp = PartitionEntry(app, PARTITION_APP)
    size = papp.size
    if papp.size == 0:
        size = papp.default_size
        extra_size = 0
        for name, value in config.items():
            entry = PartitionEntry(value, name)
            is_extra = entry not in default_partitions
            if name != PARTITION_APP and is_extra:
                extra_size += (
                    entry.size if entry.size != 0 else (entry.default_size or 0)
                )
        if extra_size != 0:
            # shrink partition space to extra
            size -= int(extra_size / 2)
        # align to 0x10000
        size &= ~(app_partition_align - 1)

    _check_partition_align(papp, app_partition_align)

    for item in [PartitionSubTypes.OTA_0, PartitionSubTypes.OTA_1]:
        entry = PartitionEntry(item)
        config[entry.name] = app.copy()
        config[entry.name][CONF_PARTITION_TYPE] = entry.tid
        config[entry.name][CONF_PARTITION_SUBTYPE] = entry.sid
        config[entry.name][CONF_PARTITION_SIZE] = size

    if PARTITION_APP in config:
        del config[PARTITION_APP]

    return config


def _default_partitions_entries(config: dict) -> list[PartitionEntry]:
    return [
        PartitionEntry(item)
        for item in (
            ESP_IDF_DEFAULT_PARTITIONS
            if CORE.using_esp_idf
            else ARDUINO_DEFAULT_PARTITIONS
            if CORE.using_arduino
            else []
        )
    ]


def apply_default_partitions(config: dict):
    default_partitions = _default_partitions_entries(config)

    for name, value in config.items():
        entry = PartitionEntry(value, name)
        if entry in default_partitions:
            default_partitions.remove(entry)

    # now add default partitions
    for entry in default_partitions:
        config[entry.name] = {}

    return config


def remove_partitions(config: dict):
    to_remove = []
    for name, value in config.items():
        size = value.get(CONF_PARTITION_SIZE, None)
        if size is not None and size == 0:
            to_remove.append(name)
    for name in to_remove:
        del config[name]
    return config


def _apply_partition_offsets(partitions: list[PartitionEntry]):
    partitions = sorted(partitions, key=lambda x: x.order)

    has_offset = 0
    app_index = -1
    for index, entry in enumerate(partitions):
        if entry.offset is not None:
            has_offset += 1
        if app_index == -1 and entry.is_app_type:
            app_index = index
    if has_offset != 0 and has_offset != len(partitions):
        raise cv.Invalid("No one or all partitons offsets must be defined")
    if app_index == -1:
        raise cv.Invalid("No app partition found")

    offset = 0x10000
    for entry in reversed(partitions[0:app_index]):
        offset = offset - entry.size
        entry.offset = offset

    offset = 0x10000
    for entry in partitions[app_index:]:
        entry.offset = offset
        offset = offset + entry.size


def apply_partition_defaults(config: dict):
    entries: dict[list[PartitionEntry], dict] = {}
    for name, value in config.items():
        entry = PartitionEntry(value, name)

        value[CONF_PARTITION_NAME] = entry.label
        value[CONF_PARTITION_TYPE] = entry.type
        value[CONF_PARTITION_SUBTYPE] = entry.subtype

        if entry.offset:
            value[CONF_PARTITION_OFFSET] = entry.offset

        if entry.size == 0:
            entry.size = entry.default_size
        value[CONF_PARTITION_SIZE] = entry.size

        if entry.flags is None and entry == PartitionSubTypes.NVS_KEYS:
            value[CONF_PARTITION_FLAGS] = PartitionFlags.READONLY.name.lower()
        elif entry.flags:
            value[CONF_PARTITION_FLAGS] = entry.flags

        if CORE.using_arduino:
            entries[entry] = value

    if len(entries) > 0:
        _apply_partition_offsets(entries.keys())
        for entry, value in entries.items():
            value[CONF_PARTITION_OFFSET] = entry.offset

    return config


def validate_partitions_final(config: dict):
    entries: list[PartitionEntry] = []
    for name, value in config.items():
        entry = PartitionEntry(value, name)
        if entry in entries:
            raise cv.Invalid(f"Duplicate partition {name}")
        entries.append(entry)
        if entry.size == 0:
            raise cv.Invalid(f"Invalid partition {name} size: {entry.size}")
    return config


def final_validate_config(config):
    if DOMAIN in CORE.data:
        partitons = {
            PartitionEntry(value, name): name for name, value in config.items()
        }
        for name, value in CORE.data[DOMAIN].items():
            entry = PartitionEntry(value, name)
            if entry in partitons:
                conf = config[partitons[entry]]
                if entry.size != 0 and conf.get(CONF_PARTITION_SIZE, None) is None:
                    conf[CONF_PARTITION_SIZE] = entry.size
            else:
                config[name] = {CONF_PARTITION_SIZE: entry.size}

    partitons = config
    partitons = validate_partitions(partitons)
    partitons = apply_default_partitions(partitons)
    partitons = remove_partitions(partitons)
    partitons = apply_app_partitions(partitons)
    partitons = apply_partition_defaults(partitons)
    partitons = validate_partitions_final(partitons)

    return partitons
