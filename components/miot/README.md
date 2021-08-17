# MIOT BLE platform

The implementation is quite different from ESPHome Xiaomi platform, but developed by MI specs, more clear and without magic and now under heavy development to conform need of other devices.

Support easily debugging Xiaomi BLE messages and adding devices by `lambda`.

```yaml
miot:
  on_miot_advertise:
    - mac_address: "54:EF:44:E2:26:AC"
      bindkey: "7d4811c43c6fa9f11bb3fe24c817f2b0"
      # enable debug BLE messages
      debug: true
      then:
        lambda: |-
          ESP_LOGW("main", "on_miot_advertise, miid: %d", x.id);
```

The `x` is a labmda variable witch [BLEObject](miot.h#L15)
