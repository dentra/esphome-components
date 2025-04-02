# Xiaomi Mijia Night Light 2 (MJYD02YL)

> [!WARNING]
>
> This component is deprecated now. Please use [miot_motion](../miot_motion/) instead.

<img src="miot_mjyd02yla.png" alt="MJYD02YL" width="200"/>

> This componint at beta state, please leave feedback.

> You could automatically get bindkey from Xiaomi Cloud by configuring `xiaomi_account` property in [MIOT](../miot/) platform.

Sample configuration:

```yaml
external_components:
  - source: github://dentra/esphome-components

binary_sensor:
  - platform: miot_mjyd02yla
    # String (Required), device MAC-address.
    mac_address: !secret device_mac_address
    # String, (Optional), device bind key. Will use [xiaomi_account](../miot/) if absent to automaticalLy get the bindkey.
    bindkey: !secret device_bind_key
    # String, (Optional), the name of binary sensor. Reports no motion on first non 0 of idle_time.
    name: Motion
    # BinarySensor (Optional), Light intensity: on - strong light, off - weak light.
    light:
      name: Light
    # Sensor (Optional), Time in seconds of inactivity.
    #  Additionally fires 1-3 times when no motion detected or detected for a long time. It will report 0 immediately
    #  after no motion detected and some reports with seconds of inactivity.
    idle_time:
      name: Idle Time
    # Sensor (Optional), Battery Level, %.
    battery_level:
      name: Battery Level
    # Sensor (Optional), Battery Voltage, V.
    battery_voltage:
      name: Battery Voltage
```
