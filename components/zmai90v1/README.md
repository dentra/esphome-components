# ZMAi-90 Energy Meter

The `zmai90v1` sensor platform allows you to use `ZMAi-90` energy meters sensors with `ESPHome`.

> This page refers to version V1 (V9821) of the `ZMAi-90`, which has been out of stock for a while. For using the newer V2 (TYWE3S) see [zmai90.yaml](../../zmai90.yaml) or for V3 (WB3S) look a solution [here](https://community.home-assistant.io/t/help-with-zmai-90-and-esphome-or-tasmota/308554/38?u=dentra) (use external_component source github://dentra/esphome@tuya-raw-fix-standalone) and please vote for [PR](https://github.com/esphome/esphome/pull/1812) to include patch to ESPHome.

```yaml
# Example configuration entry
...
external_components:
  - source: github://dentra/esphome-components
...
uart:
  rx_pin: GPIO3
  tx_pin: GPIO1
  baud_rate: 9600
  parity: EVEN

switch:
  # All the options of zmai90v1 platform are optional
  - platform: zmai90v1
    name: Switch
    # Restore mode, enum (Optional): Control how the switch attempts to restore state on bootup.
    # For restoring on ESP8266s, also see esp8266_restore_from_flash in the esphome section.
    #  * ALWAYS_ON (Default)- Always initialize as ON on bootup.
    #  * ALWAYS_OFF - Always initialize as OFF on bootup.
    #  * RESTORE_DEFAULT_ON - Attempt to restore state and default to ON.
    #  * RESTORE_DEFAULT_OFF - Attempt to restore state and default to OFF if not possible to restore.
    restore_mode: ALWAYS_ON
    # main switch pin, GPIOPin, default: GPIO12
    switch_pin: GPIO12
    # button pin, GPIOPin, default: GPIO13
    button_pin: GPIO13
    # button, BinarySensor
    button:
      name: Button
      # automatically bind button to the main switch, boolean, default: true
      # works only with conjunction with button
      bind_to_switch: true
    # energy sensor in kWh, Sensor
    energy:
      name: Energy
    # voltage sensor in V, Sensor,
    voltage:
      name: Voltage
    # current sensor in A, Sensor,
    current:
      name: Current
    # active power in W, Sensor
    active_power:
      name: Active Power
    # active power in VAR, Sensor
    reactive_power:
      name: Reactive Power
    # active power in VA, Sensor
    apparent_power:
      name: Apparent Power
    # power factor, Sensor
    power_factor:
      name: Power Factor
    # frequency in Hz, Sensor
    frequency:
      name: Frequency
    update_interval: 30s
```
