# Energy Statistics

Gather statistics for:
* today
* yesterday
* week
* month

> You can take a look at sample of usage of Energy* components in configuartion for `ZMAi-90` energy meter based on `TYWE3S`: [zmai90.yaml](../zmai90.yaml)

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
```

## Configuration variables:
* **id** (*Optional*, [ID](https://esphome.io/guides/configuration-types.html#config-id)): Manually specify the ID used for code generation.
* **total** (**Required**, [ID](https://esphome.io/guides/configuration-types.html#config-id)): The ID of the total power sensor.
* **energy_today** (*Optional*, Sensor):
  * Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
* **energy_yesterday** (*Optional*, Sensor):
  * Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
* **energy_week** (*Optional*, Sensor):
  * Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
* **energy_month** (*Optional*, Sensor):
  * Any options from [Sensor](https://esphome.io/components/sensor/index.html#config-sensor).
