import logging

from esphome.core import CORE
from esphome.helpers import write_file_if_changed

from .const import PARTITIONS_FILENAME
from .entry import PartitionEntry


def generate_csv(partitions: list[PartitionEntry], log: logging.Logger) -> None:
    max_name_len = 0
    for entry in partitions:
        max_name_len = max(max_name_len, len(entry.label))

    name_prefix = "# Name"
    name_prefix = name_prefix.ljust(max_name_len)

    line = f"{name_prefix}, Type, SubType , Offset  , Size    , Flags"
    line_len = len(line)
    if log.isEnabledFor(logging.DEBUG):
        log.info("%s # Order", line.ljust(line_len))
    else:
        log.info(line)
    output = f"{line}\n"

    for entry in partitions:
        name = entry.label.ljust(max_name_len)

        size = f"0x{int(entry.size):06X}"
        offset = f"0x{int(entry.offset):06X}" if entry.offset else ""
        flags = entry.flags or ""

        line = f"{name}, {entry.type:4s}, {entry.subtype:8s}, {offset:8s}, {size:8s}, {flags}"

        if log.isEnabledFor(logging.DEBUG):
            log.info("%s # %s", line.ljust(line_len), entry.order)
        else:
            log.info(line)

        output += f"{line}\n"

    write_file_if_changed(CORE.relative_build_path(PARTITIONS_FILENAME), output)
