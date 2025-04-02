# Universal Xiaomi Toothbrush

Already tested and works with:

- Mijia Sonic T700 (k0918.toothbrush.t700)
- Xiaomi Electric Toothbrush T700 (k0918.toothbrush.t700i)

Let me know if your toothbrush works with this component, or send miot events from your device so I can add support for it.

Sample configuration:

```yaml
external_components:
  - source: github://dentra/esphome-components

binary_sensor:
  - platform: miot_toothbrush
    # String (Required), device MAC-address.
    mac_address: !secret device_mac_address
    # String, (Optional), device bind key. Can be obtained automatically (see miot platform configuration).
    bindkey: !secret device_bind_key
    # String, (Required), the name of the sensor.
    name: Brush occupancy
    # Sensor, (Optional), the score of brushing the sensor.
    score:
      name: Score
    # Sensor, (Optional), the brushing time the sensor.
    brushing_time:
      name: Brushing Time
    # Sensor, (Optional), the consumable level the sensor.
    consumable_level:
      name: Consumable Level
    # Sensor, (Optional), the battery level the sensor.
    battery_level:
      name: Battery Level
```

## Adding new device to component

Configure [miot](../miot/) component and send me events from your toothbrush.

```yaml
miot:
 ...
  on_miot_advertise:
    then:
      lambda: |-
        ESP_LOGW("miot_event", "miid: 0x%04x, data: %s", x.id, format_hex_pretty(x.data.data(), x.data.size()).c_str());
 ...
```
