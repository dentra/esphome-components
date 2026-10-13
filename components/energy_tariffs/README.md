# Energy Tariffs

Adds tariff support to your energy meter.

> You can take a look at sample of usage of Energy\* components in configuration for `ZMAi-90` energy meter based on `TYWE3S`: [zmai90.yaml](../zmai90.yaml)

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
- **holidays** (_Optional_, List): The list of quoted holiday dates in `"DD.MM"` (every year) or `"YYYY-MM-DD"` (exact date) format. See [Days of week and holidays](#days-of-week-and-holidays).
- **holiday_as** (_Optional_, string or int): The day of week (`MON`..`SUN` or `1`..`7`, `1` is Sunday) used for tariff selection on holidays. Defaults to `SUN`.
- **holiday** (_Optional_, [ID](https://esphome.io/guides/configuration-types.html#config-id)): The ID of a binary sensor, when it is `on` the current day is a holiday. Works together with `holidays`.
- **on_tariff** (_Optional_, Action): An automation to perform when the tariff was changed. See [energy_tariffs.on_tariff](#energy_tariffs.on_tariff).
- **on_before_tariff** (_Optional_, Action): An automation that will be performed one second ahead of the tariff change time. See [energy_tariffs.on_before_tariff](#energy_tariffs.on_before_tariff).

## Tariff Sensor

Provide energy consumption for specified time period.

> One tariff (usually peak) is required to be default (without `time` and `days_of_week`).

### Configuration variables:

- **time** (List): The list of time periods in `HH:MM-HH:MM` format.
- **days_of_week** (_Optional_, string or List): The days of week when the tariff is active: names (`MON`..`SUN`), numbers (`1`..`7`, `1` is Sunday) and ranges, e.g. `MON-FRI`, `FRI-MON`, `SAT, SUN` or `[SAT, SUN]`. Without `time` the tariff is active the whole day. Defaults to every day.
- **service** (_Optional_, string): The name of service to correct value published to `Home Assistant`.
- Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).

## Days of week and holidays

The tariff is selected in the following order:

1. The first tariff in the list matching both `time` and `days_of_week`.
2. The default tariff.

On a holiday (date from `holidays` or `holiday` binary sensor is `on`) the day of week is replaced with `holiday_as`.

The day of week is checked for the current moment, so the `22:00-07:00` period with `days_of_week: MON-FRI` is not active on Saturday from 00:00 to 07:00.

> Holiday dates must be quoted: unquoted `12.10` is parsed by YAML as number `12.1`.

Spain PVPC 2.0TD: P1/P2 on workdays, P3 at night, on weekends and national holidays:

```yaml
- platform: energy_tariffs
  total: total
  holidays: ["01.01", "06.01", "01.05", "15.08", "12.10", "01.11", "06.12", "08.12", "25.12"]
  tariffs:
    - name: P1
      time: ["10:00-14:00", "18:00-22:00"]
      days_of_week: MON-FRI
    - name: P2
      time: ["08:00-10:00", "14:00-18:00", "22:00-00:00"]
      days_of_week: MON-FRI
    - name: P3
```

Australia TOU: peak/shoulder on workdays, off-peak at night and on weekends:

```yaml
- platform: energy_tariffs
  total: total
  tariffs:
    - name: Peak
      time: ["07:00-09:00", "17:00-20:00"]
      days_of_week: MON-FRI
    - name: Shoulder
      time: ["09:00-17:00", "20:00-22:00"]
      days_of_week: MON-FRI
    - name: Off-Peak
```

Holidays from Home Assistant [Workday](https://www.home-assistant.io/integrations/workday/) integration. Configure the Workday sensor with all 7 days as workdays, so it is `off` on holidays only:

```yaml
binary_sensor:
  - platform: homeassistant
    id: holiday
    entity_id: binary_sensor.workday_sensor
    filters:
      - invert:

sensor:
  - platform: energy_tariffs
    total: total
    holiday: holiday
    tariffs:
      ...
```

Until the binary sensor receives its first state after boot, only `holidays` list is used. When Home Assistant disconnects, the last received state is kept.

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

This automation will be triggered when the current tariff changes. The new tariff sensor is available in [lambdas](https://esphome.io/guides/automations.html#config-lambda) as `x` (`Sensor *`).

It is also triggered once after boot, as soon as the time is synchronized and the initial tariff is selected. The tariff selection takes into account `time_offset`, `days_of_week` and holidays.

```yaml
sensor:
  - platform: energy_tariffs
    ...
    on_tariff:
      - text_sensor.template.publish:
          id: current_tariff
          state: !lambda return x->get_name();
```

Compare `x` with tariff IDs to perform different actions:

```yaml
    on_tariff:
      - if:
          condition:
            lambda: return x == id(offpeak);
          then:
            - switch.turn_on: boiler
          else:
            - switch.turn_off: boiler
```

## energy_tariffs.on_before_tariff

This automation will be triggered once, one second before the tariff changes. It has no arguments.

Use it to request a fresh `total` value from the meter, so the energy consumed up to this moment is counted by the current tariff. E.g. for Tuya based meters (like `ZMAi-90`) send the query data point command:

```yaml
sensor:
  - platform: energy_tariffs
    ...
    on_before_tariff:
      - uart.write: [0x55, 0xaa, 0x00, 0x08, 0x00, 0x00, 0x07]
```

> Changes caused by the `holiday` binary sensor can not be predicted, so the automation is not triggered for them.

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
