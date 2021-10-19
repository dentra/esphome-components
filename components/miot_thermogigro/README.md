# Universal Xiaomi Thermometer Hygrometer

This component possibly should work with any Xiaomi or sub-brands (Mijia, Yeelight, Qingping, MiaoMiaoce etc...) devices wich can measure temperature and/or humidity.

* ClearGrass Bluetooth Hygrothermograph (CGG1)
* Xiaomi Mi Temperature and Humidity Monitor 2 (LYWSD03MMC)
* Xiaomi Mijia Temperature And Humidity Electronic Watch (LYWSD02MMC)
* Xiaomi MiaoMiaoce Smart Clock Temperature Fnd Humidity Meter E-Inc (MHO-C303)
* Xiaomi MiaoMiaoce Digital Bluetooth Thermometer Hygrometer (MHO-C401)

and others... Please let me know your device to add to this list.

> You colud automaticaly get bindkey from Xiaomi Cloud by configuring `xiaomi_account` property in [MIOT](../miot/) platform.

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

sensor:
  - platform: miot_thermogigro
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # String, (Optional), device bind key. Will use "xiaomi_account" from "miot" if absent to automaticaly get the bindkey.
    bindkey: "device-bin-key"
    # String, (Optional), the name of sensor.
    name: "Temperature"
    # Sensor (Optional), Humidity, %
    humidity:
      name: "Humidity"
    # Sensor (Optional), Battey Level, %
    battery_level:
      name: "Battery Level"
```

Now signal strength of a BLE device (RSSI) is not a part of this component, but you could easily add it:
```yaml
sensor:
  - platform: ble_rssi
    mac_address: "device-mac-address"
    name: "RSSI"
```

