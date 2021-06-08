# xiaomi_mccgq02hl

This component is copy of https://github.com/adamklis/esphome/tree/dev/esphome/components/xiaomi_mccgq02hl with small fixes
and allow compilation using great `ESPHome` `external_components` feature.

Also equires `xiaomi_ble` from https://github.com/adamklis/esphome/

Sample configuration:
```yaml
esphome:
  platform: ESP32
  board: esp32dev
  name: test_xiaomi_mccgq02hl

external_components:
  - source: github://dentra/esphome-components
    components: [xiaomi_mccgq02hl]
  - source: github://adamklis/esphome
    components: [xiaomi_ble]

esp32_ble_tracker:

binary_sensor:
  - platform: xiaomi_mccgq02hl
    name: door sensor
    mac_address: "5c:02:72:7a:03:bb"
    bindkey: "9468e743ceb55cc656aafe224e65a5bd"
    battery_level:
      name: door sensor battery level
    light:
      name: door sensor light
    opening:
      name: door sensor open
```
