# Xiaomi Flood Sensor (SJWS01LM)


Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

binary_sensor:
  - platform: miot_sjws01lm
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # String, (Optional), device bind key
    bindkey: "device-bin-key"
    # String, (Optional), the name of binary sensor
    name: "SJWS01LM Water Leak"
    # Sensor (Optional), Battey Level, %
    battery_level:
      name: "RTCGQ02LM Battery Level"
```
