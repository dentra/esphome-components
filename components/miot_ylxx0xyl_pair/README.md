# Obtain beakonkey for Yeelight remote controllers.

Supported models are:

- Remote (YLYK01YL)
- Dimmer (YLKG07YL/YLKG08YL)
- Monitor Light Bar Remote (MJGDYK01YL)
- Bathroom Heater Remote (YLYB01YL-BHFRC)
- Fan Remote (YLYK01YL-VENFAN)
- Ventilator Fan Remote (YLYK01YL-FANCL)

Configure `text_sensor` with `miot_ylxx0xyl_pair` platform and MAC-address of your remote then flash ESP32 device and put remote in pairing mode, wait about minute and copy obtained beakonkey from configured sensor.

Sample configuration:

```yaml
external_components:
  - source: github://dentra/esphome-components

ble_client:
  - mac_address: !secret device_mac_address
    id: remote_ble_client_id

text_sensor:
  - platform: miot_ylxx0xyl_pair
    name: miot_ylxx0xyl_pair Bindkey
    ble_client_id: remote_ble_client_id
    # Sesnor (Optional), sensor do display version.
    version:
      name: miot_ylxx0xyl_pair Version
    # uint16 (Optional), product id to pair. defaults to 0, means auto for one of 0x0153, 0x03B6, 0x15CE, 0x03BF, 0x04E6, 0x068E
    #                    can be used to obtain beakonkey from unsupported devices.
    #product_id: 0
```
