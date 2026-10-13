# Xiaomi Water Leak Detector 2 (SJWS02LM)

<img src="miot_sjws02lm.png" alt="SJWS02LM" width="200"/>

> You could automatically get bindkey from Xiaomi Cloud by configuring `xiaomi_account` property in [MIOT](../miot/) platform.

The device has two independent sets of contacts: bottom (water on the floor) and top (dripping water).
The main binary sensor is `on` when water is detected by any of them.

Sample configuration:

```yaml
external_components:
  - source: github://dentra/esphome-components

binary_sensor:
  - platform: miot_sjws02lm
    # String (Required), device MAC-address.
    mac_address: !secret device_mac_address
    # String, (Optional), device bind key. Will use "xiaomi_account" from "miot" if absent to automatically get the bindkey.
    bindkey: !secret device_bind_key
    # String, (Optional), the name of binary sensor, water is detected by bottom or top contacts
    name: Water Leak
    # Binary Sensor (Optional), water is detected by bottom contacts
    bottom:
      name: Water Leak Bottom
    # Binary Sensor (Optional), water is detected by top contacts
    top:
      name: Water Leak Top
    # Sensor (Optional), Battery Level, %
    battery_level:
      name: Battery Level
    # Sensor (Optional), Battery Voltage, V
    battery_voltage:
      name: Battery Voltage
```

Now signal strength of a BLE device (RSSI) is not a part of this component, but you could easily add it:

```yaml
sensor:
  - platform: ble_rssi
    mac_address: !secret device_mac_address
    name: RSSI
```
