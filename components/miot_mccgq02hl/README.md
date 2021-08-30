# Xiaomi Mi Smart Home Door/Window Sensor 2 (MCCGQ02HL)

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

binary_sensor:
  - platform: miot_mccgq02hl
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # String, (Optional), device bind key
    bindkey: "device-bin-key"
     # String, (Optional), the name of binary sensor
    name: "MCCGQ02HL Door/Window Sensor"
    # BinarySensor (Optional), Light intensivity: on - strong light, off - weak light
    light:
      name: "MCCGQ02HL Light"
    # BinarySensor (Optional), Alert enabled when door/window has opened a long (device configured) time
    alert:
      name: "MCCGQ02HL Alert"
    # Sensor (Optional), Battey Level, %
    battery_level:
      name: "MCCGQ02HL Battery Level"
    # Sensor (Optional), Signal strength, dB
    rssi:
      name: "RTCGQ02LM RSSI"
```
