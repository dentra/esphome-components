# Universal Xiaomi Motion

This component possibly should work with any Xiaomi or sub-brands (Mijia, Yeelight, Qingping, MiaoMiaoce etc...) devices which can detect motion.

- Xiaomi Mijia Night Light 2 (MJYD02YL)
- Xiaomi Mi Smart Home Occupancy Sensor 2 (RTCGQ02LM)
- Qingping Motion & Ambient Light Sensor (CGPR1)

and others... Please let me know your device to add to this list.

Sample configuration:

```yaml
external_components:
  - source: github://dentra/esphome-components

xiaomi_account:
  username: !secret xiaomi_username
  password: !secret xiaomi_password
  servers: cn

binary_sensor:
  - platform: miot_motion
    # String, (Required), Device MAC-address.
    mac_address: !secret device_mac_address
    # String, (Optional), The name of this binary sensor.
    name: Motion
    # BinarySensor (Optional), Light intensity: on - strong light, off - weak light.
    #  Note that it will work only if the sensor report it or report illuminance.
    light: Light
    # Sensor (Optional), Illuminance in lux.
    #  Note that it will work only if the sensor report it.
    illuminance: Illuminance
    # Sensor (Optional), Time in seconds of inactivity.
    #  Note that it will work only if the sensor report it.
    idle_time: Idle Time
    # Sensor (Optional), Battery Level, %.
    #  Note that it will take a while until sensor report it.
    battery_level: Battery Level
```

Advanced configuration:

```yaml
binary_sensor:
  - platform: miot_motion
    # String, (Required), Device MAC-address.
    mac_address: !secret device_mac_address
    # String, (Optional), Device bind key. Will use [xiaomi_account](../miot/) if absent to automatically get the bindkey.
    bindkey: !secret device_bind_key"
    # Time, (Optional), Sets the time after which a sensor is reset its state.
    #   After each `motion detected` advertisement, the timer starts counting down again.
    #   Setting this option to 0 seconds will turn this resetting behavior off.
    #   Default: 0s (disabled)
    #   Note that motion sensors also sends advertisements themselves that can overrule this setting.
    timeout: 5s
    name: Motion
    light:
      name: Light
    illuminance:
      name: Illuminance
    idle_time:
      name: Idle Time
    battery_level:
      name: Battery Level
    battery_voltage:
      name: Battery Voltage
```
