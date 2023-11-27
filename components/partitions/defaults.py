from .types import PartitionSubTypes

ARDUINO_DEFAULT_PARTITIONS = [
    PartitionSubTypes.NVS,
    PartitionSubTypes.OTA,
    PartitionSubTypes.EEPROM,
    PartitionSubTypes.SPIFFS,
]

ESP_IDF_DEFAULT_PARTITIONS = [
    PartitionSubTypes.OTA,
    PartitionSubTypes.PHY,
    PartitionSubTypes.NVS,
]

ESP_IDF_DEFAULT_SIZES = {
    PartitionSubTypes.OTA: 0x2000,
    PartitionSubTypes.PHY: 0x1000,
    PartitionSubTypes.NVS: 0x6D000,
    PartitionSubTypes.COREDUMP: 0x10000,
    PartitionSubTypes.EFUSE: 0x2000,
    PartitionSubTypes.NVS_KEYS: 0x1000,
}

ARDUINO_DEFAULT_SIZES = {
    **ESP_IDF_DEFAULT_SIZES,
    PartitionSubTypes.NVS: 0x5000,
    PartitionSubTypes.EEPROM: 0x1000,
    PartitionSubTypes.SPIFFS: 0x00F000,
}

ESP_IDF_PARTITION_ORDER = {
    PartitionSubTypes.OTA: -100,
    PartitionSubTypes.PHY: -50,
}

ARDUINO_PARTITION_ORDER = {
    PartitionSubTypes.NVS: -150,
    PartitionSubTypes.OTA: -100,
    PartitionSubTypes.PHY: -50,
    PartitionSubTypes.EEPROM: PartitionSubTypes.TEST.order + 1,
}
