# OTAX

Allow to enable/disable OTA at runtime.

> WARNING: Works only with ESPHome below 2026.3.0, since 2026.3.0 the ESPHome OTA component can't be extended anymore.

```yaml
# Example configuration entry.
 ...
external_components:
  - source: github://dentra/esphome-components
 ...
otax:
  # Required, switch to enable/disable OTA.
  enabled:
    name: $name OTA Enabled
```

## ESPHome 2026.3.x - 2026.4.x

The same can be done without this component by enabling/disabling the OTA component loop:

```yaml
ota:
  - platform: esphome
    id: ota_esphome

switch:
  - platform: template
    name: OTA Enabled
    optimistic: true
    restore_mode: ALWAYS_ON
    icon: mdi:cloud-upload-outline
    entity_category: diagnostic
    turn_on_action:
      - lambda: id(ota_esphome).enable_loop();
    turn_off_action:
      - lambda: id(ota_esphome).disable_loop();
```

## ESPHome 2026.5.0 and up

There is no way to stop OTA component at runtime: it re-enables its loop on every incoming connection.
