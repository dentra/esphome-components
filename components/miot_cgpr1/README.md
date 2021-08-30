# Xiaomi Mi Smart Home Occupancy Sensor 2 (RTCGQ02LM)

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
    name: "RTCGQ02LM Motion Sensor 2"
    # BinarySensor (Optional), device configured timeout
    timeout:
      name: "RTCGQ02LM Timeout"
    # Sensor (Optional), Time in seconds of inactivity
    idle_time:
      name: "RTCGQ02LM Idle Time"
    # BinarySensor (Optional), Light intensivity: on - strong light, off - weak light
    light:
      name: "RTCGQ02LM Light"
    # Sensor (Optional), Battey Level, %
    battery_level:
      name: "RTCGQ02LM Battery Level"
    # Sensor (Optional), Signal strength, dB
    rssi:
      name: "RTCGQ02LM RSSI"
```
