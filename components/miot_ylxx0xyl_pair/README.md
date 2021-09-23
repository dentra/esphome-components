# Obtain beakonkey for Yeelight remote controllers.

Supported models are:
* Remote (YLYK01YL)
* Dimmer (YLKG07YL/YLKG08YL)
* Bathroom Heater Remote (YLYB01YL-BHFRC)
* Fan Remote (YLYK01YL-VENFAN)
* Ventilator Fan Remote (YLYK01YL-FANCL)

Configure `text_sensor` with `miot_ylxx0xyl_pair` platform and MAC-address of your remote then flash ESP32 device and put remote in pairing mode, wait about minute and copy obtained beakonkey from configured sensor.

Sample configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components

# do not include ble_client if you already configure it for other purpose
ble_client:
  - mac_address: "00:00:00:00:00:00" # workaraund to include ble_client but not use them

text_sensor:
  - platform: miot_ylxx0xyl_pair
    name: "miot_ylxx0xyl_pair Status"
    # String (Required), device MAC-address.
    mac_address: "device-mac-address"
    # Sesnor (Optional), sensor do display bindkey.
    beakonkey:
      name: "miot_ylxx0xyl_pair Bindkey"
    # Sesnor (Optional), sensor do display version.
    version:
      name: "miot_ylxx0xyl_pair Version"
```
