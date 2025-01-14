# Energy Statistics

Gather statistics for:

- today
- yesterday
- week
- month
- year

> You can take a look at sample of usage of Energy\* components in configuartion for `ZMAi-90` energy meter based on `TYWE3S`: [zmai90.yaml](../zmai90.yaml)

```yaml
# Example configuration entry
 ...
external_components:
  - source: github://dentra/esphome-components
 ...
sensor:
  - platform: "energy_statistics"
    total: total
    energy_today:
      name: "$name Energy Today"
    energy_yesterday:
      name: "$name Energy Yesterday"
    energy_week:
      name: "$name Energy Week"
    energy_month:
      name: "$name Energy Month"
    energy_year:
      name: "$name Energy Year"
```

## Configuration variables:

- **id** (_Optional_, [ID](https://esphome.io/guides/configuration-types.html#config-id)): Manually specify the ID used for code generation.
- **total** (**Required**, [ID](https://esphome.io/guides/configuration-types.html#config-id)): The ID of the total power sensor.
- **energy_today** (_Optional_, Sensor):
  - Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
- **energy_yesterday** (_Optional_, Sensor):
  - Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
- **energy_week** (_Optional_, Sensor):
  - Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
- **energy_month** (_Optional_, Sensor):
  - Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
- **energy_year** (_Optional_, Sensor):
  - Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
