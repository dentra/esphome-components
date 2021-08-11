# Xiaomi Mi Smart Home Door/Window Sensor 2 (MCCGQ02HL)

This component is based on https://github.com/adamklis/esphome/tree/dev/esphome/components/xiaomi_mccgq02hl
and allow compilation using great `ESPHome` `external_components` feature.

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
  - platform: xiaomi_mccgq02hl
    mac_address: "E4:AA:EC:34:77:53"
    bindkey: "574adf25535c9752b9bb6c31b0833442"
    name: "MCCGQ02HL Door/Window Sensor"
    light:
      name: "MCCGQ02HL Light"
    battery_level:
      name: "MCCGQ02HL Battery Level"
```
