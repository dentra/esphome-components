# `ESPHome` components

[![Open in Visual Studio Code](https://open.vscode.dev/badges/open-in-vscode.svg)](https://open.vscode.dev/dentra/esphome-components)

A collection of my ESPHome components.

To use this repository you should confugure it inside your yaml-configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components
```

> You can take a look at sample of usage of those components in configuartion for `ZMAi-90` energy meter based on `TYWE3S`: [zmai90.yaml](zmai90.yaml)

## [Energy Monitoring](components/energy_monitoring/)
Turn additional measurements features in your energy meter.

## [Energy Statistics](components/energy_statistics/)
Gather energy statistics.

## [Energy Tariffs](components/energy_tariffs/)
Get support of tariffs right in your energy meter.

## [Startup Sensor](components/startup/)
Uptime sensor based on timestamp.

## [Backup](components/backup/)
Save your config back to firmware and download it.

## [ZMAi-90 v1](components/zmai90v1/)
Turn your ZMAi-90 on V9821 chip into ESPHome device.

## [Xiaomi Mi Smart Home Door/Window Sensor 2 (MCCGQ02HL)](components/miot_mccgq02hl/)
New Xiaomi BLE device.

## [Xiaomi Flood Sensor (SJWS01LM)](components/miot_sjws01lm/)
New Xiaomi BLE device.

## [Xiaomi Mi Smart Home Occupancy Sensor 2 (RTCGQ02LM)](components/miot_rtcgq02lm/)
New Xiaomi BLE device.
