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
    battery_level:
      name: "SJWS01LM Battery Level"
    rssi:
      name: "SJWS01LM RSSI"
```
