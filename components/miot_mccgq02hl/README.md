# Xiaomi Mi Smart Home Door/Window Sensor 2 (MCCGQ02HL)

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

binary_sensor:
  - platform: miot_mccgq02hl
    mac_address: "E4:AA:EC:34:77:53"
    bindkey: "574adf25535c9752b9bb6c31b0833442"
    name: "MCCGQ02HL Door/Window Sensor"
    light:
      name: "MCCGQ02HL Light"
    alert:
      name: "MCCGQ02HL Alert"
    battery_level:
      name: "MCCGQ02HL Battery Level"
    rssi:
      name: "MCCGQ02HL RSSI"
```
