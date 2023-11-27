from enum import Enum, IntEnum, StrEnum


class PartitionTypes(IntEnum):
    APP = 0x00
    DATA = 0x01

    @property
    def id(self) -> int:
        return self.value

    @property
    def type(self) -> str:
        return self.name.lower()


class PartitionSubTypes(Enum):
    FACTORY = (PartitionTypes.APP, 0x00, None)
    OTA_0 = (PartitionTypes.APP, 0x10, "app0")
    OTA_1 = (PartitionTypes.APP, 0x11, "app1")
    OTA_2 = (PartitionTypes.APP, 0x12, "app2")
    OTA_3 = (PartitionTypes.APP, 0x13, "app3")
    OTA_4 = (PartitionTypes.APP, 0x14, "app4")
    OTA_5 = (PartitionTypes.APP, 0x15, "app5")
    OTA_6 = (PartitionTypes.APP, 0x16, "app6")
    OTA_7 = (PartitionTypes.APP, 0x17, "app7")
    OTA_8 = (PartitionTypes.APP, 0x18, "app8")
    OTA_9 = (PartitionTypes.APP, 0x19, "app9")
    OTA_10 = (PartitionTypes.APP, 0x1A, "app10")
    OTA_11 = (PartitionTypes.APP, 0x1B, "app11")
    OTA_12 = (PartitionTypes.APP, 0x1C, "app12")
    OTA_13 = (PartitionTypes.APP, 0x1D, "app13")
    OTA_14 = (PartitionTypes.APP, 0x1E, "app14")
    OTA_15 = (PartitionTypes.APP, 0x1F, "app15")
    TEST = (PartitionTypes.APP, 0x20, None)
    OTA = (PartitionTypes.DATA, 0x00, "otadata")
    PHY = (PartitionTypes.DATA, 0x01, "phy_init")
    NVS = (PartitionTypes.DATA, 0x02, None)
    COREDUMP = (PartitionTypes.DATA, 0x03, None)
    NVS_KEYS = (PartitionTypes.DATA, 0x04, "nvs_key")
    EFUSE = (PartitionTypes.DATA, 0x05, "efuse_em")
    UNDEFINED = (PartitionTypes.DATA, 0x06, None)
    ESPHTTPD = (PartitionTypes.DATA, 0x80, None)
    FAT = (PartitionTypes.DATA, 0x81, None)
    SPIFFS = (PartitionTypes.DATA, 0x82, None)
    LITTLEFS = (PartitionTypes.DATA, 0x83, None)
    EEPROM = (PartitionTypes.DATA, 0x99, None, True)

    @property
    def is_custom(self) -> bool:
        return len(self.value) > 3 and self.value[3]

    @property
    def id(self) -> int:
        return self.value[1]

    @property
    def subtype(self) -> str:
        return f"0x{self.id:02x}" if self.is_custom else self.name.lower()

    @property
    def type(self) -> str:
        return self.value[0].type

    @property
    def type_id(self) -> int:
        return self.value[0].id

    @property
    def label(self) -> str:
        return self.value[2] if self.value[2] is not None else self.name.lower()

    @property
    def order(self) -> int:
        return PartitionSubTypes.order_for(self.type_id, self.id)

    @staticmethod
    def order_for(type_id: int, subtype_id: int):
        return type_id * 0x100 + subtype_id


class PartitionFlags(StrEnum):
    READONLY = "readonly"
    ENCRYPTED = "encrypted"
