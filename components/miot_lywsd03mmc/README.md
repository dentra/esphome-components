# Xiaomi Mi Temperature and Humidity Monitor 2 (LYWSD03MMC)

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

sensor:
  - platform: miot_lywsd03mmc
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # String, (Optional), device bind key
    bindkey: "device-bin-key"
    # String, (Optional), the name of sensor
    name: "LYWSD03MMC Temperature"
    # Sensor (Optional), Humidity, %
    humidity:
      name: "LYWSD03MMC Humidity"
    # Sensor (Optional), Battey Level, %
    battery_level:
      name: "LYWSD03MMC Battery Level"
```

Now signal strength of a BLE device (RSSI) is not a part of this component, but you could easily add it:
```yaml
sensor:
  - platform: ble_rssi
    mac_address: "device-mac-address"
    name: "LYWSD03MMC RSSI"
```

thanks to @xrays72
