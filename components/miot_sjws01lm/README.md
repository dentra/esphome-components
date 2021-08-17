# Xiaomi Flood Sensor (SJWS01LM)


Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

binary_sensor:
  - platform: miot_sjws01lm
    mac_address: "54:EF:44:E1:5B:B8"
    bindkey: "0f873478d18c786973429000a56f0852"
    name: "SJWS01LM Water Leak"
    # Sensor (Optional), Battey Level, %
    battery_level:
      name: "RTCGQ02LM Battery Level"
    # Sensor (Optional), Signal strength, dB
    rssi:
      name: "RTCGQ02LM RSSI"
```
