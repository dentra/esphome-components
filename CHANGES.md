# 2022-09-28
* [+] new component etl

# 2022-08-25
* [+] new component crash_info

# 2021-10-26
* [*] `zmai90.yaml`: add throttle_average filter to energy_monitoring sensors
* [-] `energy_tariffs`: remove save_to_flash_interval parameter to use preferences.flash_write_interval
* [-] `energy_statistics`: remove save_to_flash_interval parameter to use preferences.flash_write_interval

# 2021-11-15
* [+] new component `miot_ylkg0xyl`
* [+] add battery voltage to all supported miot components
* [-] remove dupliate log messages from miot button events

# 2021-11-03
* [+] `miot_thermogigro`: add support `XMWSDJ04MMC`

# 2021-10-27
* [+] new component `miot_kettle`
* [+] new component `miot_xmmfo1jqd`

# 2021-10-26
* [*] `energy_tariffs`: new save_to_flash_interval parameter
* [*] `energy_statistics`: new save_to_flash_interval parameter

# 2021-10-18
* [+] new component `miot_thermogigro`
* [*] deprecate `miot_cgg1`, now use `miot_thermogigro`
* [*] deprecate `miot_lywsd03mmc`, now use `miot_thermogigro`
* [*] deprecate `miot_mhoc303`, now use `miot_thermogigro`

# 2021-10-18
* [+] new component `miot_mhoc303`
* [*] update to esphome 2021.9.3
* [*] support esphome 2021.10 beta

# 2021-10-13
* [-] remove ble_client include from `miot_explorer`
* [+] new component `miot_client`
* [*] `miot_ylxx0xyl_pair` switched to `miot_client`

# 2021-09-27
* [+] new component `miot_mjyd02yla` Xiaomi Mijia Night Light 2 (MJYD02YL)

# 2021-09-24
* [+] new component `miot_ylai003` Yeelight Remote Control 1S (YLAI003)

# 2021-09-22
* [+] new component `miot_ylxx0xyl_pair`: obtain beakonkey for Yeelight Remote (YLYK01YL), Dimmer (YLKG07YL/YLKG08YL), Bathroom Heater Remote (YLYB01YL-BHFRC), Fan Remote (YLYK01YL-VENFAN) and Ventilator Fan Remote (YLYK01YL-FANCL)

# 2021-09-21
* [*] fixed #1 rtcgq02lm documentation copy-paset error
* [*] fixed #2 sensor.h unavailable
* [*] update to esphome 2021.9.1

# 2021-09-17
* [*] fix math errors in energy monitoring

# 2021-09-09
* [+] miot: add automatic bindkey extraction from Xiaomi Cloud

# 2021-08-31
* [+] miot: add miot explorer component
* [+] miot: add legacy ecrypted mibeacons support
* [+] miot: add Xiaomi Mi Temperature and Humidity Monitor 2 (LYWSD03MMC)
