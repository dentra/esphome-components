# Qingping Motion & Ambient Light Sensor (CGPR1)

<img src="miot_cgpr1.png" alt="CGPR1" width="200"/>

> You colud automaticaly get bindkey from Xiaomi Cloud by configuring `xiaomi_account` property in [MIOT](../miot/) platform.

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

binary_sensor:
  - platform: miot_cgpr1
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # String, (Optional), device bind key
    bindkey: "device-bin-key"
    # String, (Optional), the name of binary sensor
    name: "CGPR1 Motion"
    # Sensor (Optional), Illuminance, lux
    illuminance:
      name: "CGPR1 Illuminance"
    # Sensor (Optional), Time in seconds of inactivity
    idle_time:
      name: "CGPR1 Idle Time"
    # BinarySensor (Optional), Light intensivity: on - strong light, off - weak light
    light:
      name: "CGPR1 Light"
    # Sensor (Optional), Battey Level, %
    battery_level:
      name: "CGPR1 Battery Level"
```

Now signal strength of a BLE device (RSSI) is not a part of this component, but you could easily add it:
```yaml
sensor:
  - platform: ble_rssi
    mac_address: "device-mac-address"
    name: "CGPR1 RSSI"
```

thanks to @xrays72
