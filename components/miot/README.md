# MIOT BLE platform

The implementation is quite different from ESPHome Xiaomi platform, but by MI specs, more clear and without magic and now under heavy development to conform need of other devices.

Support addidig devices by `lambda`

```yaml
miot:
 on_miot_advertise:
    - mac_address: $mac_motion
      bindkey: $key_motion
       debug: true
       then:
         lambda: |-
           ESP_LOGW("$node_name", "on_miot_advertise - $mac_motion");
```

The `x` labmda variable contains [BLEObject](miot.h#L15)
