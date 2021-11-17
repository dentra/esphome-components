# Xiaomi Mijia Night Light 2 (MJYD02YL)

<img src="miot_mjyd02yla.png" alt="MJYD02YL" width="200"/>

> This componint at beta state, please leave feedback.

> You colud automaticaly get bindkey from Xiaomi Cloud by configuring `xiaomi_account` property in [MIOT](../miot/) platform.

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

binary_sensor:
  - platform: miot_mjyd02yla
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # String, (Optional), device bind key. Will use [xiaomi_account](../miot/) if absent to automaticaly get the bindkey.
    bindkey: "device-bin-key"
    # String, (Optional), the name of binary sensor. Reports no motion on first non 0 of idle_time.
    name: "$name Motion"
    # BinarySensor (Optional), Light intensivity: on - strong light, off - weak light.
    light:
      name: "$name Light"
    # Sensor (Optional), Time in seconds of inactivity.
    #  Additionally fires 1-3 times when no motion detected or detected for a long time. It will report 0 imedeatly
    #  after no motion detected and some reports with seconds of inactivity.
    idle_time:
      name: "$name Idle Time"
    # Sensor (Optional), Battey Level, %.
    battery_level:
      name: "$name Battery Level"
    # Sensor (Optional), Battey Voltage, V.
    battery_voltage:
      name: "$name Battery Voltage"
```
