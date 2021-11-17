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
    # String, (Optional), device bind key. Will use "xiaomi_account" from "miot" if absent to automaticaly get the bindkey.
    bindkey: "device-bin-key"
    # String, (Optional), the name of binary sensor
    name: "$name Motion"
    # Boolean, (Optional), detect no motion as soon as possible (Usually when idle_time = 0). Default is false
    detect_no_motion_asap: false
    # Sensor (Optional), Illuminance, lux
    illuminance:
      name: "$name Illuminance"
    # Sensor (Optional), Time in seconds of inactivity
    idle_time:
      name: "$name Idle Time"
    # BinarySensor (Optional), Light intensivity: on - strong light, off - weak light
    light:
      name: "$name Light"
    # Sensor (Optional), Battey Level, %
    battery_level:
      name: "$name Battery Level"
    # Sensor (Optional), Battey Voltage, V
    battery_voltage:
      name: "$name Battery Voltage"
```

Now signal strength of a BLE device (RSSI) is not a part of this component, but you could easily add it:
```yaml
sensor:
  - platform: ble_rssi
    mac_address: "device-mac-address"
    name: "$name RSSI"
```

thanks to @xrays72
