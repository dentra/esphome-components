# MIOT BLE platform

The implementation is quite different from ESPHome Xiaomi platform, but developed by MI specs, more clear and without magic and now under heavy development to conform need of other devices.

Supports automatic bindkey extraction from Xiaomi Cloud, see [Xiaomi account](#xiaomi-account). Thanks to [AlexxIT](https://github.com/AlexxIT) for [XiaomiGateway3](https://github.com/AlexxIT/XiaomiGateway3) and [Piotr Machowski](https://github.com/PiotrMachowski) for [Xiaomi-cloud-tokens-extractor](https://github.com/PiotrMachowski/Xiaomi-cloud-tokens-extractor).

Supports easily debugging Xiaomi BLE beacons.

Supports adding new devices via lambda.

```yaml
external_components:
  - source: github://dentra/esphome-components

miot:
  # Map (Optional), bindkey automatically will be requested for any miot component. Requires internet connection when compiling.
  xiaomi_account:
    # String (Optional), name of the stored login session, required only when you use several Xiaomi accounts.
    username: !secret xiaomi_username
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

## Xiaomi account

Login into Xiaomi Cloud is performed with QR code, password is not used anymore (`password` option is deprecated and ignored).

When there is no stored login session, the build prints a link to the QR code image and a login link, then waits up to 5 minutes for confirmation. When running from the console, the QR code is also printed as text. You can:

- scan the QR code with the Mi Home app (in the ESPHome dashboard open the QR code image link first);
- or log in to https://account.xiaomi.com in a browser and then open the login link in the same browser.

After successful login the session is stored in `.esphome/xiaomi_account/` (`/data/xiaomi_account/` for Home Assistant add-on) and reused by next builds without any confirmation. Keep this file private, it gives full access to your Xiaomi account. Delete it to log in again.

Devices and bindkeys are cached in the build directory for `update_interval`. If login fails, outdated cache is used.

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
