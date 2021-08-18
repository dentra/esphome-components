# Yeelight Remote Control Smart LED Ceiling Light (YLYK01YL)

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

sensor:
  - platform: miot_ylyk01yl
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # String, (Optional), device bind key
    bindkey: "device-bin-key"
    # Sensor (Optional), Battey Level, %
    battery_level:
      name: "YLYK01YL Battery Level"
    # Sensor (Optional), Signal strength, dB
    rssi:
      name: "YLYK01YL RSSI"
    # List, Automation (Optional), The automation activated when you clicked specified button
    on_click:
      # String (Required), The button name (case insensitive):
      #     on, off, dimmable or sun or moon, plus or +, moonlight or M, minus or -
      - button: "on"
        # List, Any automation
        then:
          lambda: // do something
      - button: "off"
        # List, Any automation
        then:
          - homeassistant.service:
              service: notify.html5
              data:
                title: Off button was pressed
    # List, Automation (Optional), The automation activated when you log pressed specified button
    on_long_press:
      # String (Required), The button name (case insensitive):
      #     on, off, dimmable or sun or moon, plus or +, moonlight or M, minus or -
      - button: "+"
      # List, Any automation
        then:
          lambda: // do something
      - button: "-"
      # List, Any automation
        then:
          - homeassistant.service:
              service: notify.html5
              data:
                title: Minus button was long pressed
```

thanks to @Slavissy
