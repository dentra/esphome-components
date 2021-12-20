# `ESPHome` components

[![Open in Visual Studio Code][open-in-vscode-shield]][open-in-vscode]
[![PayPal.Me][paypal-me-shield]][paypal-me]

[open-in-vscode-shield]: https://open.vscode.dev/badges/open-in-vscode.svg
[open-in-vscode]: https://open.vscode.dev/dentra/esphome-components

[paypal-me-shield]: https://img.shields.io/static/v1.svg?label=%20&message=PayPal.Me&logo=paypal
[paypal-me]: https://paypal.me/dentra0

A collection of my ESPHome components.

To use this repository you should confugure it inside your yaml-configuration:
```yaml
external_components:
  - source: github://dentra/esphome-components
```

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

## [MIOT Platform](components/miot/)
The MIOT Platform is alternative implementation of Xiaomi BLE protocol.
At this moment there are list of supported device:
 * [Xiaomi Mi Smart Home Door/Window Sensor 2 (MCCGQ02HL)](components/miot_mccgq02hl/)
 * [Xiaomi Flood Sensor (SJWS01LM)](components/miot_sjws01lm/)
 * [Xiaomi Mi Smart Home Occupancy Sensor 2 (RTCGQ02LM)](components/miot_rtcgq02lm/)
 * [Xiaomi ClearGrass Bluetooth Hygrothermograph (CGG1)](components/miot_cgg1/)
 * [Yeelight Remote Control Smart LED Ceiling Light (YLYK01YL)](components/miot_ylyk01yl/)
 * [Qingping Motion & Ambient Light Sensor (CGPR1)](components/miot_cgpr1/)
 * [Xiaomi Mi Temperature and Humidity Monitor 2 (LYWSD03MMC)](components/miot_lywsd03mmc/)
 * [Yeelight Remote Control 1S (YLAI003)](components/miot_ylai003/)
 * [Xiaomi Mijia Night Light 2 (MJYD02YL)](components/miot_mjyd02yla/)
 * [Xiaomi MiaoMiaoce Smart Clock Temperature And Humidity Meter E-Inc (MHO-C303)](components/miot_mhoc303/)
 * [Universal Xiaomi Thermometer Hygrometer](components/miot_thermogigro/)
 * [Universal Xiaomi Kettle](components/miot_kettle/)
 * [Xiaomi Mijia bluetooth Smart Rubik's Cube (XMMFO1JQD)](components/miot_xmmfo1jqd/)
 * [Xiaomi Mijia Temperature And Humidity Electronic Watch (LYWSD02MMC)](components/miot_thermogigro/)
 * [Xiaomi MiaoMiaoce Digital Bluetooth Thermometer Hygrometer (MHO-C401)](components/miot_thermogigro/)
 * [Xiaomi MiaoMiaoce Digital Temperature and Humidity Monitor (XMWSDJ04MMC)](components/miot_thermogigro/)
 * [Yeelight Smart Bluetooth Wireless Dimmer Wall Light Switch Remote Control (YLKG08YL/YLKG07YL)](components/miot_ylkg0xyl/)

And growing...

> Now with automatic bindkey extraction from Xiaomi Cloud.

## [MIOT Explorer](components/miot_explorer/)
MIOT Platform based component to explore new BLE devices from Xiaomi ecosystem.

## [MIOT Yeelight Remote pair](components/miot_ylxx0xyl_pair/)
Obtain beakonkey for Yeelight Remote (YLYK01YL), Dimmer (YLKG07YL/YLKG08YL), Bathroom Heater Remote (YLYB01YL-BHFRC), Fan Remote (YLYK01YL-VENFAN) and Ventilator Fan Remote (YLYK01YL-FANCL).

## [Electrolux Water Heater](https://github.com/dentra/esphome-ewh)
Control Electrolux Water Heater and possibly other boilers (Ballu, Zanussi) using uart protocol.

### Your thanks
If this project was useful to you, you can [buy me](https://paypal.me/dentra0) a Cup of coffee :)
