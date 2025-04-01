# Partitions

An ESP32 component to customize partitions from ESPHome configuration.

> WARNING: This is an advanced component and you need to understand what you are doing.

For usage instructions please see examples with comments bellow:

```yaml
external_components:
  - source: github://dentra/esphome-components
    components: [partitions]

partitions:
  # Partition entry with name 'app'.
  # The 'app' is a special partition entry, it used to define two ota
  # partitions, size of those partitons is calculated based on flash
  # size and extra (non-default) partitions.
  app:

  # Known partition entry names:
  #   factory, ota_0 ... ota_15/app_0 ... app_15, test, phy/phy_init, nvs,
  #   nvs_keys/nvs_key, ota/otadata, coredump, undefined, esphttpd, fat,
  #   spiffs, littlefs, efuse/efuse_em, eeprom.
  ota:

  # If you do not need the default partition entry you can set it size to 0.
  spiffs: 0

  # create coredump partition entry with 64K size.
  coredump: 0x10000

  # name of the partition entry
  nvs:
    # type: app, data or number 0-254 (or as hex 0x00-0xFE)
    type: data
    # subtype: factory, ota_1 ... ota_15, test, phy, nvs, nvs_keys, ota, coredump,
    #                undefined, esphttpd, fat, spiffs, or number 0-254 (or as hex 0x00-0xFE)
    subtype: nvs
    # size, required filed for all non-default partition entries
    size: 0x5000
    # offset field: in most cases calculated automatically
    offset: 0x9000

  phy_init:
    # override default partition entry name
    name: my_phy
```

## Minimal example

```yaml
external_components:
  - source: github://dentra/esphome-components
    components: [partitions]

# Enable cusom partitions generation, use defaults and print partition entry.
partitions:
  # you should define at least one partition entry, otherwise default esphome partition table will be used.
  app:
```

```yaml
external_components:
  - source: github://dentra/esphome-components
    components: [partitions]

# Add littlefs partition to default partitions and remove spiffs.
partitions:
  # remove arduino default spiffs partition
  spiffs: 0
  # add additional littlefs partition
  littlefs: 0xF000
```

```yaml
external_components:
  - source: github://dentra/esphome-components
    components: [partitions]

# Use defaults and add coredump partition with default size.
partitions:
  coredump:
```

## Full example

You can completely control all of partition entry fields.

```yaml
external_components:
  - source: github://dentra/esphome-components
    components: [partitions]

partitions:
  nvs:
    type: data
    subtype: nvs
    size: 0x5000
    offset: 0x9000
  otadata:
    type: data
    subtype: ota
    size: 0x2000
    offset: 0xE000
  app0:
    type: app
    subtype: ota_0
    size: 0x1C0000
    offset: 0x10000
  app1:
    type: app
    subtype: ota_1
    size: 0x1C0000
    offset: 0x1D0000
  eeprom:
    type: data
    subtype: 0x99
    size: 0x1000
    offset: 0x390000
  spiffs:
    type: data
    subtype: spiffs
    size: 0xF000
    offset: 0x391000
```
