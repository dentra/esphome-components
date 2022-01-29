# EraClean Refrigerator Odor Eliminator Max (CWBS-01)

> This component is in early beta!

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

ota:
  on_begin:
    then:
      - lambda: id(sw_ble_client).turn_off();

miot:
  xiaomi_account:
    username: !secret xiaomi_username
    password: !secret xiaomi_password
    servers: cn
    update_interval: 24h

ble_client:
  - mac_address: $mac_deodorizer
    id: ble_client_deodorizer

switch:
  - platform: ble_client
    id: sw_ble_client
    ble_client_id: ble_client_deodorizer
    name: cwbs01 Enabled

binary_sensor:
  - platform: miot_cwbs01
    ble_client_id: ble_client_deodorizer
    name: "${name} State"
    # Optional
    version:
       name: "${name} Version"
    # Optional, switch power
    power:
      name: "${name} Power"
    # Optional, switch cycle
    cycle:
      name: "${name} Cycle"
    # Optional, select mode
    mode:
      name: "${name} Mode"
    # Optional, select scene
    scene:
      name: "${name} Scene"
    # Optional, state of power plug
    charging:
      name: "${name} Charging"
    # Optional, battery level %
    battery_level:
      name: "${name} Battery Level"
    # Optional, device state
    error:
      name: "${name} Error"

```

