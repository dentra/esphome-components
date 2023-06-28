# Miot Explorer

You can easily explore device MI beacon information.
Just create text sensor, wait some time and grab information about objects and their data in automaticaly created sensors.
Then based on this information you could create PR to new componet or using [MIOT Platform](../miot/) and lambda catch sensors data yourself.

> New sensors are not created in web server automatically, so you should refresh web brower page sometimes.

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

text_sensor:
  - platform: miot_explorer
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # String, (Optional), device bind key
    bindkey: "device-bind-key"
    # String, (Required), the name of sensor
    name: None # use esphome.friendly_name
    # Sensor (Optional), Humidity, %
    battery_level:
      name: "Battery Level"
    # Sensor (Optional), Battey Voltage, V
    battery_voltage:
      name: "Battery Voltage"
    # additional optional sensors:
    consumable:
      name: "Consumable"
```
