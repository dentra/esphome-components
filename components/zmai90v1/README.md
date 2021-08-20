# ZMAi-90 Energy Meter

The `zmai90v1` sensor platform allows you to use `ZMAi-90` energy meters sensors with `ESPHome`.

> This page refers to version V1 (V9821) of the `ZMAi-90`, which has been out of stock for a while. For using the newer V2 (TYWE3W) see [zmai90.yaml](../../zmai90.yaml) or V3 (WB3S) please fill the request (I have solution).

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
