[esp32_ble_tracker]: https://esphome.io/components/esp32_ble_tracker/
[binary_sensor]: https://esphome.io/components/binary_sensor/
[sensor]: https://esphome.io/components/sensor/
[ID]: https://esphome.io/guides/configuration-types#config-id
[Automation]: https://esphome.io/guides/automations#automation
[PayPal]: https://paypal.me/dentra0
[Card2Card]: https://www.tinkoff.ru/cf/3dZPaLYDBAI
[Boosty]: https://boosty.to/dentra

# Qingping BLE platform

This platform implements parsing Qingping BLE advertisements and exposing them as sensors or binary sensors.

Supported known data points:
 * Temperature
 * Humidity
 * Battery
 * Door/Window/Opening
 * Door/Window/Opening problem
 * Pressure
 * Motion
 * Illuminance
 * Light
 * PM 2.5
 * PM 10
 * Carbon Dioxide (CO2)

Also supports any other type via automation or feature request.

Provide explorier feature to get data without knowing MAC-address or supported data points.

Minimal sample configuration:
```yaml
# Load component implementation and code generation helpers from github.
external_components:
  - source: github://dentra/esphome-components
    components: [qingping, codegen]

# Define qingping BLE listener on specified MAC-address.
qinping:
  mac_address: "11:22:33:44:55:66"

sensor:
  - platform: qingping
    type: co2
    name: CO2

binary_sensor:
  - platform: qingping
    type: light
    name: Light

```

> [!NOTE]
> If you do not receiving Qingping BLE advertisements on WiFi device you may need to connect it to the cloud.

> [!IMPORTANT]
> To compile properly this component requires C++17 with GNU extensions and up.
> It automatically changes environment to `gnu++17` to support it.

## Configuring Qingping Hub.

The Qingping platform is using [esp32_ble_tracker] so you able to configure it first.

The Qingping platform maintains a list of hubs, each of which is responsible for
listening to a specific device. The exception is a hub with explorer or with verbose mode,
it can listen any device.

```yaml
# Load component implementation and code generation helpers from github.
external_components:
  - source: github://dentra/esphome-components
    components: [qingping, codegen]

qingping:
  - id: qp_th
    # MAC-address is required for specific defice.
    mac_address: "11:22:33:44:55:66"
  - id: qp_air_monitor
    # MAC-address is required for specific defice.
    mac_address: "11:22:33:44:55:77"
    # Log all processed packets
    verbose: true
  - id: qp_new_device
    # MAC-address is required for specific defice.
    mac_address: "11:22:33:44:55:88"
    # Log all processed packets
    verbose: true
    on_state:
      - lambda: |-
          // cast to known structure that work with battery level data type
          auto *battery = x.cast<QPDataBatteryLevel>();
          // check that our variable fits this data point
          if (battery) {
            ESP_LOGD("main", "Battery Level %f (%u)",
              battery->get_battery_level(), battery->raw_battery_level);
          }
      - lambda: |-
          // check for known data point QPID::BATTERY
          if (x.type == QPID::BATTERY) {
              // cast to known structure that work with battery level data type
              auto *battery = x.cast<QPDataBatteryLevel>();
              ESP_LOGD("main", "Battery Level %f (%u)",
                battery->get_battery_level(), battery->raw_battery_level);
          }
      - lambda: |-
          // check for unknown 0x55 data point
          if (x.raw_type == 0x55) {
            // cast to universal structure that work with any data type
            auto *type55 = x.cast<QPDataRaw>();
            ESP_LOGD("main", "Type 55 value is %u", type55->u16);
          }

  - id: qp_explorer
    # listen all mac-addresses and automatically create sensors/binary_sensor for known data points
    explorer:
      # do not create and publish temperature sensor
      temperature: False
      # do not create and publish motion binary_sensor
      motion: False
      # create battery level sensor with name "Battery"
      battery_level: Battery
    # Log all processed packets
    verbose: true

sensor:
  - platform: qingping
    # attach sensor to qp_th hub
    qingping_id: qp_th
    type: battery
    name: Battery Level

  - platform: qingping
    # attach sensor to qp_air_monitor hub
    qingping_id: qp_air_monitor
    type: carbon_dioxide
    name: Carbon Dioxide
```

### Configuration variables:
 * `id` (Optional, [ID]): Manually specify the ID for this hub.
 * `mac_address` (Optional, MAC Address): The MAC address to track for this hub.
 Note that mac_address is required if `explorer` or `verbose` is not present.

### Automations:
 * `on_state` (Optional, [Automation]): An automation to perform when a new BLE device state change is received.

### Advanced options:
 * `use_beacon_mac` (Optional, boolean): If true uses MAC-address from the beacon. Default: False.
 * `verbose` (Optional, boolean): If true enables verbose output to log. . Default: False.
 * `explorer` (Optional, Schema): Can automatically creates sensors and binary sensors based on

### explorer variables:

All explorer variables are present by default, you can exclude unnecessary ones by setting them to None or False.

 * `temperature` (Optional, string): The name of created temperature sensor. Default: Temperature.
 * `humidity` (Optional, string): The name of created humidity sensor. Default: Humidity.
 * `battery_level` (Optional, string): The name of created battery level sensor. Default: Battery Level.
 * `door` (Optional, string): The name of created door binary sensor. Default: Door.
 * `door_left_open` (Optional, string): The name of created door left open binary sensor. Default: Door Left Open.
 * `pressure` (Optional, string): The name of created pressure sensor. Default: Pressure.
 * `motion` (Optional, string): The name of created motion binary sensor. Default: Motion.
 * `illuminance` (Optional, string): The name of created illuminance sensor. Default: Illuminance.
 * `light` (Optional, string): The name of created light binary sensor. Default: Light.
 * `pm10` (Optional, string): The name of created pm 10 sensor. Default: PM 10.
 * `pm25` (Optional, string): The name of created pm 2.5 sensor. Default: PM 2.5.
 * `co2` (Optional, string): The name of created co2 sensor. Default: Carbon Dioxide.

## Configuring platform `binary_sensor`

The qingping binary sensor platform creates a binary sensor from a qingping data
point and requires Qingping Hub with `mac_address` to be configured.

You can create the binary sensor as follows:

```yaml
binary_sensor:
  - platform: qingping
    type: light
    name: Light
```

### Configuration variables:
 * `type` (Rquired, string) - type of the binary sensor, see below.
 * `qingping_id` (Optional, [ID]) - id of qingping hub. You can skip this parameter if you have only one hub.
 * `id` (Optional, [ID]): Manually specify the ID for this binary sensor.
 * `name` (Optional, string): The name of the binary sensor.
 * All other options from [binary_sensor].

### Configuration types:
 * `door`
 * `window`
 * `opening`
 * `door_problem`
 * `window_problem`
 * `opening_problem`
 * `motion`
 * `light`

## Configuring platform `sensor`

The qingping sensor platform creates a sensor from a qingping data
point and requires Qingping Hub with `mac_address` to be configured.

You can create the sensor as follows:

```yaml
sensor:
  - platform: qingping
    type: co2
    name: CO2
```

### Configuration variables:
 * `type` (Rquired, string) - type of the sensor, see below.
 * `qingping_id` (Optional, [ID]) - id of qingping hub. You can skip this parameter if you have only one hub.
 * `id` (Optional, [ID]): Manually specify the ID for this sensor.
 * `name` (Optional, string): The name of the sensor.
 * All other options from [sensor].

### Configuration types:

 * `temperature`
 * `humidity`
 * `battery_level` / `battery`
 * `pressure`
 * `illuminance` / `light`
 * `packet`
 * `pm25` / `pm_25` / `pm_2_5`
 * `pm10` / `pm_10`
 * `co2` / `carbon_dioxide`


## Your thanks
If this project was useful to you, you can buy me a Cup of coffee via
[PayPal], [Card2Card], [Boosty] subscripion or or just star the project :)
