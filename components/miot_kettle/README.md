# Universal Xiaomi Kettle

This component possibly should work with any Xiaomi or sub-brands (Mijia, Yeelight, Qingping, MiaoMiaoce etc...) devices which can boil water.

* Xiaomi Viomi Smart Kettle Bluetooth Pro V-SK152 (V-SK152)

and others... Please let me know your device to add to this list.

> You colud automaticaly get bindkey from Xiaomi Cloud by configuring `xiaomi_account` property in [MIOT](../miot/) platform.

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

binary_sensor:
  - platform: miot_kettle
    # String (Required), device MAC-address.
    mac_address: "$device_mac_address"
    # String, (Optional), device bind key. Will use "xiaomi_account" from "miot" if absent to automaticaly get the bindkey.
    bindkey: "$device_bindkey"
    # String, (Optional), the name of binary sensor
    name: "Kettle Water Leak"
    # Sensor (Optional), Temperature, °C
    temperature:
      name: "Kettle Temperature"
```
