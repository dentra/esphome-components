# Qingping Motion & Ambient Light Sensor (CGPR1)

> [!WARNING]
>
> This component is deprecated now. Please use [miot_motion](../miot_motion/) instead.

> [!TIP]
>
> You could also configure this device via [Qingping](../qingping/) platform.

<img src="miot_cgpr1.png" alt="CGPR1" width="200"/>

> You could automatically get bindkey from Xiaomi Cloud by configuring `xiaomi_account` property in [MIOT](../miot/) platform.

Sample configuration:

```yaml
external_components:
  - source: github://dentra/esphome-components

binary_sensor:
  - platform: miot_cgpr1
    # String (Required), device MAC-address.
    mac_address: !secret device_mac_address
    # String, (Optional), device bind key. Will use "xiaomi_account" from "miot" if absent to automatically get the bindkey.
    bindkey: !secret device_bind_key
    # String, (Optional), the name of binary sensor.
    name: Motion
    # Boolean, (Optional), detect no motion as soon as possible (Usually when idle_time = 0). Default is false.
    detect_no_motion_asap: false
    # Sensor (Optional), Illuminance, lux.
    illuminance:
      name: Illuminance
    # Sensor (Optional), Time in seconds of inactivity.
    #   Additionally fires 1-3 times when no motion detected or detected for a long time. It will report 0 immediately
    #   after no motion detected, 60 when mo motion is 1 min ago and 300 when mo motion is 5 min ago.
    idle_time:
      name: Idle Time
    # BinarySensor (Optional), Light intensity: on - strong light, off - weak light.
    light:
      name: Light
    # Sensor (Optional), Battery Level, %.
    battery_level:
      name: Battery Level
    # Sensor (Optional), Battery Voltage, V.
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

thanks to @xrays72
