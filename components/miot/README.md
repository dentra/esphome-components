# MIOT BLE platform

The implementation is quite different from ESPHome Xiaomi platform, but developed by MI specs, more clear and without magic and now under heavy development to conform need of other devices.

Supports automatic bindkey extraction from Xiaomi Cloud. Thanks to [Piotr Machowski](https://github.com/PiotrMachowski) for his great [Xiaomi-cloud-tokens-extractor](https://github.com/PiotrMachowski/Xiaomi-cloud-tokens-extractor).

Supports easily debugging Xiaomi BLE beacons.

Supports adding new devices via lambda.

```yaml
external_components:
  - source: github://dentra/esphome-components

miot:
  # Map (Optional), bindkey automatically will be requested for any miot component. Requires internet connection when compiling.
  xiaomi_account:
    # String (Required), Xiaomi Cloud account username
    username: !secret xiaomi_username
    # String (Required), Xiaomi Cloud account password
    password: !secret xiaomi_password
    # List (Optional), List of servers to request. Default: cn, de, us, ru, tw, sg, in, i2
    servers: cn
    # Time (Optional), The interval the information will be requested again from Xiaomi servers. Default: 1d
    update_interval: 4h

  on_miot_advertise:
    # String (Required), device MAC-address
    mac_address: !secret device_mac_address
    # String, (Optional), device bind key
    bindkey: !secret device_bind_key
    # enable debug BLE messages, DO NOT USE IN PRODUCTION
    debug: true
    then:
      lambda: // empty action
```

You can use automation to operate with [BLEObject](miot_object.h#L86) via `x` variable in lambda.

```yaml
external_components:
  - source: github://dentra/esphome-components

miot:
  on_miot_advertise:
    # String (Required), device MAC-address.
    mac_address: !secret device_mac_address
    # String, (Optional), device bind key
    bindkey: !secret device_bind_key
    # enable debug BLE messages, DO NOT USE IN PRODUCTION
    debug: true
    then:
      lambda: |-
        ESP_LOGW("miot_event", "miid: 0x%04x, data: %s", x.id, format_hex_pretty(x.data.data(), x.data.size()).c_str());
```

> Also you could use [MIOT Explorer](../miot_explorer/) to grab sensors information.

## Adding new device to component

Configure [miot](../miot/) component and send me events from your toothbrush.
