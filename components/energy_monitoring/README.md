# Energy Monitoring

This componet allows you to get additional measurements features with you favorite energy meter.
It calculates `Apparent Power`, `Reactive Power` and `Power Factor`.

> You can take a look at sample of usage of Energy* components in configuartion for `ZMAi-90` energy meter based on `TYWE3S`: [zmai90.yaml](../zmai90.yaml)

The configuration is very simple. Look at a sample below:
```yaml
# Example configuration entry
...
external_components:
  - source: github://dentra/esphome-components
...
sensor:
  - platform: "energy_monitoring"
    power: power_id
    voltage: voltage_id
    current: current_id
    apparent_power:
      name: "$name Apparent Power"
    reactive_power:
      name: "$name Reactive Power"
    power_factor:
      name: "$name Power Factor"
    wait_time: 500
```

## Configuration variables:

* **id** (*Optional*, [ID](https://esphome.io/guides/configuration-types.html#config-id)): Manually specify the ID used for code generation.
* **power** (**Required**, [ID](https://esphome.io/guides/configuration-types.html#config-id)): The ID of the power sensor from your power meter.
* **voltage** (**Required**, [ID](https://esphome.io/guides/configuration-types.html#config-id)): The ID of the voltage sensor from your power meter.
* **current** (*Optional*, [ID](https://esphome.io/guides/configuration-types.html#config-id)): The ID of the current sensor from your power meter.
* **apparent_power** (*Optional*, Sensor): The sensor with calculated apparent power.
  * Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
* **reactive_power** (Optional, Sensor): The sensor with calculated reactive power.
  * Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
* **power_factor** (Optional, Sensor): The sensor with calculated power factor.
  * Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
* **wait_time** (*Optional*, int): Time in milliseconds to wait before previous calculations. Defaults to 500ms. Set to 0 to disable it.

## Bonus: Works even with pulse meters.

Just set `power` sensor, but `voltage` make as static or use it from your smart socket.

Example with static voltage:
```yaml
# Example configuration entry
sensor:
  - platform: "energy_monitoring"
    power: power_id
    voltage: voltage_id
    apparent_power:
      name: "$name Apparent Power"
    reactive_power:
      name: "$name Reactive Power"
    power_factor:
      name: "$name Power Factor"
  - platform: "template"
    id: voltage
    lambda: return 230.0;
```

Example with voltage from Home Assistant:
```yaml
# Example configuration entry
sensor:
  - platform: "energy_monitoring"
    power: power_id
    voltage: voltage_id
    apparent_power:
      name: "$name Apparent Power"
    reactive_power:
      name: "$name Reactive Power"
    power_factor:
      name: "$name Power Factor"
  - platform: "homeassistant"
    id: voltage
    entity_id: "sensor.kitchen_plug_voltage"
    accuracy_decimals: 1
```
