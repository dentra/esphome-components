# Energy Tariffs

Adds tariff support to your energy meter.

> You can take a look at sample of usage of Energy\* components in configuration for `ZMAi-90` energy meter based on `TYWE3S`: [zmai90.yaml](../zmai90.yaml)

> Detailed description will be provided soon...

```yaml
# Example configuration entry
 ...
external_components:
  - source: github://dentra/esphome-components
 ...
- platform: energy_tariffs
    id: tariffs
    total: total
    time_offset:
      name: Time Offset
    tariffs:
      - name: Peak Tariff
        id: peak
        icon: mdi:weather-sunny
        service: peak_set
        initial_value: 10.0
      - name: Off-Peak Tariff
        id: offpeak
        icon: mdi:weather-night
        time: 23:00-07:00
        service: offpeak_set
    on_tariff:
      lambda: |-
        if (x == id(peak))
          id(current_tariff).publish_state("peak");
        else if (x == id(offpeak))
          id(current_tariff).publish_state("off-peak");
        else // should be never happen
          id(current_tariff).publish_state("unknown");
    on_before_tariff:
      - uart.write: [0x55, 0xaa, 0x00, 0x08, 0x00, 0x00, 0x07]

text_sensor:
  - platform: template
    id: current_tariff
    name: Current Tariff
    icon: mdi:theme-light-dark
    entity_category: diagnostic
```

## Configuration variables:

- **id** (_Optional_, [ID](https://esphome.io/guides/configuration-types.html#config-id)): Manually specify the ID used for code generation.
- **total** (**Required**, [ID](https://esphome.io/guides/configuration-types.html#config-id)): The ID of the total power sensor.
- **time_offset** (_Optional_, int): The number of seconds offset to synchronize with your mains time. Positive or negative. Defaults 0.
- **time_offset_service** (_Optional_, string): The name of service published to `Home Assistant`.
- **tariffs**: (**Required**, List): List of Tariff sensors. Minimum 1, maximum 4 (Let me know if you need more). See [Tariff Sensor](#Tariff_Sensor).
- **on_tariff** (_Optional_, Action): An automation to perform when the tariff was changed. See [energy_tariffs.on_tariff](#energy_tariffs.on_tariff).
- **on_before_tariff** (_Optional_, Action): An automation that will be performed one second ahead of the tariff change time. See [energy_tariffs.on_before_tariff](#energy_tariffs.on_before_tariff).

## Tariff Sensor

Provide energy consumption for specified time period.

> One tariff (usually peak) is required to be default (without time).

### Configuration variables:

- **time** (List): The list of time periods in `HH:MM-HH:MM` format.
- **service** (_Optional_, string): The name of service to correct value published to `Home Assistant`.
- Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).

## Lambda calls

From [lambdas](https://esphome.io/guides/automations.html#config-lambda), you can call several methods to do some advanced stuff.

- **get_time_offset()**: You could easily get configured time offset if you need it.
  ```yaml
  # Example configuration entry
   ...
  esphome:
    name: my_node
   ...
  external_components:
    - source: github://dentra/esphome-components
   ...
  sensor:
    - platform: template
      id: time_offset
      name: Time Offset
      icon: mdi:clock-fast
      accuracy_decimals: 0
      unit_of_measurement: "s"
      lambda: |-
        return id(tariffs).get_time_offset();
  ```
  And display it in `Home Assistant`:
  ```yaml
  # Example HA configuration entry
  sensor:
    - platform: template
      sensors:
        my_node_time:
          friendly_name: My Node Time
          icon_template: mdi:clock-outline
          value_template: >-
            {{ (as_timestamp(now()) + states('sensor.my_node_time_offset')|int) | timestamp_custom("%H:%M:%S") }}
  ```

## energy_tariffs.on_tariff

> Description will be provided soon...

## energy_tariffs.on_before_tariff

> Description will be provided soon...

## Correct values without Home Assistant service

```yaml
number:
  - platform: template
    name: peak set
    min_value: 0
    max_value: 999999
    step: 0.01
    set_action:
      lambda: id(peak).publish_state_and_save(x);
  - platform: template
    name: off-peak set
    min_value: 0
    max_value: 999999
    step: 0.01
    set_action:
      lambda: id(offpeak).publish_state_and_save(x);
```
