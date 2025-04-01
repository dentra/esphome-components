# Yeelight Smart Bluetooth Wireless Dimmer Wall Light Switch Remote Control (YLKG08YL/YLKG07YL)

Sample configuration:

```yaml
external_components:
  - source: github://dentra/esphome-components

sensor:
  - platform: miot_ylkg0xyl
    # String (Required), device MAC-address.
    mac_address: "$device_mac_address"
    # String, (Optional), device bind key. Will use "xiaomi_account" from "miot" if absent to automatically get the bindkey.
    bindkey: "$device_bindkey"
    # List, Automation (Optional), The automation activated when you make specified action
    on_short_press:
      # List, Any automation
      - then:
          - logger.log:
              format: knob was short pressed %u times
              args: ["x"]
    on_long_press:
      - then:
          - logger.log: knob was long pressed
    on_rotate_left:
      - then:
          - logger.log:
              format: knob was rotating left on %u
              args: ["x"]
    on_rotate_right:
      - then:
          - logger.log:
              format: knob was rotating right on %u
              args: ["x"]
    on_rotate_left_pressed:
      - then:
          - logger.log:
              format: knob was pressed and rotating left on %u
              args: ["x"]
    on_rotate_right_pressed:
      - then:
          - logger.log:
              format: knob was pressed and rotating right on %u
              args: ["x"]
```

thanks to @xrays72
