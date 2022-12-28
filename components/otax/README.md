# OTAX

Allow to enable/disable OTA at runtime.

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
