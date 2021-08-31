# MIOT BLE platform

The implementation is quite different from ESPHome Xiaomi platform, but developed by MI specs, more clear and without magic and now under heavy development to conform need of other devices.

Support easily debugging Xiaomi BLE beacons.

> Also you colud use [MIOT Explorer](../miot_explorer/) to grab sesnors information.

```yaml
external_components:
  - source: github://dentra/esphome-components

miot:
  on_miot_advertise:
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # String, (Optional), device bind key
    bindkey: "device-bin-key"
    # enable debug BLE messages, DO NOT USE IN PRODUCTION
    debug: true
```

Also you can use automation to operate with [BLEObject](miot.h#L15) via `x` variable in lambda.

```yaml
external_components:
  - source: github://dentra/esphome-components

miot:
  on_miot_advertise:
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # String, (Optional), device bind key
    bindkey: "device-bin-key"
    # enable debug BLE messages, DO NOT USE IN PRODUCTION
    debug: true
    then:
      lambda:
        ESP_LOGW("main", "on_miot_advertise, miid: %d", x.id);
```
