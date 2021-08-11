# Xiaomi Flood Sensor (SJWS01LM)


Sample configuration:
```yaml
esphome:
  platform: ESP32
  board: esp32dev
  name: xiaomi_mccgq02hl

external_components:
  - source: github://dentra/esphome-components
    components: [xiaomi_mccgq02hl]

esp32_ble_tracker:

binary_sensor:
  - platform: xiaomi_sjws01lm
    mac_address: "54:EF:44:E1:5B:B8"
    bindkey: "0f873478d18c786973429000a56f0852"
    name: "SJWS01LM Water Leak"
    battery_level:
      name: "SJWS01LM Battery Level"
```
