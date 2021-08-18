# Xiaomi ClearGrass Bluetooth Hygrothermograph (CGG1)

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

sensor:
  - platform: miot_cgg1
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # String, (Optional), device bind key
    bindkey: "device-bin-key"
     # String, (Optional), the name of sensor
    name: "CGG1 Temperature"
    # Sensor (Optional), Humidity, %
    humidity:
      name: "CGG1 Humidity"
    # Sensor (Optional), Battey Level, %
    battery_level:
      name: "CGG1 Battery Level"
    # Sensor (Optional), Signal strength, dB
    rssi:
      name: "CGG1 RSSI"
```
