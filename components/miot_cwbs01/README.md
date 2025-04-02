# EraClean Refrigerator Odor Eliminator Max (CWBS-01) aka EraClean Intelligent deodorizer (era.airp.cwb03)

> This component is in beta stage.

This component is using active bluetooth device connection, so do not set update interval less to save your device battery life.

Sample configuration:

```yaml
external_components:
  - source: github://dentra/esphome-components

ota:
  on_begin:
    then:
      - lambda: id(ble_client_deodorizer).set_enabled(false);

xiaomi_account:
  username: !secret xiaomi_username
  password: !secret xiaomi_password
  servers: cn
  update_interval: 24h

ble_client:
  - mac_address: $mac_deodorizer
    id: ble_client_deodorizer

binary_sensor:
  - platform: miot_cwbs01
    ble_client_id: ble_client_deodorizer
    name: State
    # Optional
    version:
      name: Version
    # Optional, switch power
    power:
      name: Power
    # Optional, switch cycle
    cycle:
      name: Cycle
    # Optional, select mode
    mode:
      name: Mode
    # Optional, select scene
    scene:
      name: Scene
    # Optional, state of power plug
    charging:
      name: Charging
    # Optional, battery level %
    battery_level:
      name: Battery Level
    # Optional, internal device state
    error:
      name: Error
    # Optional, Time: The interval that the device state should be checked. Defaults to 1 hour.
    update_interval: 1h
```
