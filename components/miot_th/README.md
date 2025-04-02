# Universal Xiaomi Thermometer Hygrometer

This component possibly should work with any Xiaomi or sub-brands (Mijia, Yeelight, Qingping, MiaoMiaoce etc...) devices which can measure temperature and/or humidity.

- ClearGrass Bluetooth Hygrothermograph (CGG1)
- Xiaomi Mi Temperature and Humidity Monitor 2 (LYWSD03MMC)
- Xiaomi Mijia Temperature And Humidity Electronic Watch (LYWSD02MMC)
- Xiaomi MiaoMiaoce Smart Clock Temperature Fnd Humidity Meter E-Inc (MHO-C303)
- Xiaomi MiaoMiaoce Digital Bluetooth Thermometer Hygrometer (MHO-C401)
- Xiaomi MiaoMiaoce Digital Temperature and Humidity Monitor (XMWSDJ04MMC)
- Mijia Smart Temperature and Humidity Monitor 3 (MJWSD05MMC)

and others... Please let me know your device to add to this list.

> You could automatically get bindkey from Xiaomi Cloud by configuring `xiaomi_account` property in [MIOT](../miot/) platform.

Sample configuration:

```yaml
external_components:
  - source: github://dentra/esphome-components

sensor:
  - platform: miot_th
    # String (Required), device MAC-address.
    mac_address: !secret device_mac_address
    # String, (Optional), device bind key. Will use "xiaomi_account" from "miot" if absent to automatically get the bindkey.
    bindkey: !secret device_bind_key
    # String, (Optional), the name of sensor.
    name: Temperature
    # Sensor (Optional), Humidity, %
    humidity:
      name: Humidity
    # Sensor (Optional), Battery Level, %
    battery_level:
      name: Battery Level
    # Sensor (Optional), Battery Voltage, V
    battery_voltage:
      name: Battery Voltage
```

Now signal strength of a BLE device (RSSI) is not a part of this component, but you could easily add it:

```yaml
sensor:
  - platform: ble_rssi
    mac_address: !secret device_mac_address
    name: RSSI
```

## Adding new device to component

Configure [miot](../miot/) component and send me events from your toothbrush.

```yaml
miot:
 ...
  on_miot_advertise:
    then:
      lambda: |-
        ESP_LOGW("miot_event", "miid: 0x%04x, data: %s", x.id, format_hex_pretty(x.data.data(), x.data.size()).c_str());
 ...
```
