# Yeelight Remote Control Smart LED Ceiling Light (YLYK01YL)

> At this moment it is not supported Xiaomi Cloud key extraction for this device. So you need to get it yourself.

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
    # List, Automation (Optional), The automation activated when you clicked specified button
    on_click:
      # String (Required), The button name (case insensitive):
      #     on, off, dimmable or sun or moon, plus or +, moonlight or M, minus or -
      - button: "on"
        # List, Any automation
        then:
          - logger.log: button ON click
    # List, Automation (Optional), The automation activated when you log pressed specified button
    on_long_press:
      # String (Required), The button name (case insensitive):
      #     on, off, dimmable or sun or moon, plus or +, moonlight or M, minus or -
      - button: "+"
      # List, Any automation
        then:
           - logger.log: button PLUS long press

```

thanks to @Slavissy
