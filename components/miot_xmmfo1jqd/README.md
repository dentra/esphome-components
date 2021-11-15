# Xiaomi Mijia bluetooth Smart Rubik's Cube (XMMFO1JQD)

<img src="miot_xmmfo1jqd.png" alt="XMMFO1JQD" width="200"/>

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

sensor:
  - platform: miot_xmmfo1jqd
    # String (Required), device MAC-address.
    mac_address: "$device_mac_address"
    # String, (Optional), device bind key. Will use "xiaomi_account" from "miot" if absent to automaticaly get the bindkey.
    bindkey: "$device_bindkey"
    # List, Automation (Optional), The automation activated when you clicked specified button
    on_twist:
      # String (Required), The button name (case insensitive): clockwise or counterclockwise
      - button: "clockwise"
        # List, Any automation
        then:
          - logger.log: twisted clockwise
      - button: "counterclockwise"
        # List, Any automation
        then:
          - logger.log: twisted counterclockwise
    # Sensor (Optional), Battey Level, % (not 100% sure that is working, please let me know it so)
    battery_level:
      name: "$name Battery Level"
    # Sensor (Optional), Battey Voltage, V (not 100% sure that is working, please let me know it so)
    battery_voltage:
      name: "$name Battery Voltage"
```

thanks to [@xrust83](https://github.com/xrust83)
