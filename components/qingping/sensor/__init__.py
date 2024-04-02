import esphome.codegen as cg
from esphome.components import sensor
from esphome.const import (
    CONF_ACCURACY_DECIMALS,
    CONF_BATTERY_LEVEL,
    CONF_CO2,
    CONF_DEVICE_CLASS,
    CONF_HUMIDITY,
    CONF_ILLUMINANCE,
    CONF_LIGHT,
    CONF_PM_2_5,
    CONF_PRESSURE,
    CONF_STATE_CLASS,
    CONF_TEMPERATURE,
    CONF_TYPE_ID,
    CONF_UNIT_OF_MEASUREMENT,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CARBON_DIOXIDE,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_ILLUMINANCE,
    DEVICE_CLASS_PM10,
    DEVICE_CLASS_PM25,
    DEVICE_CLASS_PRESSURE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_HECTOPASCAL,
    UNIT_LUX,
    UNIT_MICROGRAMS_PER_CUBIC_METER,
    UNIT_PARTS_PER_MILLION,
    UNIT_PERCENT,
)

from .. import (
    CONF_PM10,
    CONF_PM25,
    new_pc,
    qingping_ns,
    qingping_pc_ns,
    qp_final_validate_config,
)

QingpingSensor = qingping_ns.class_("QingpingSensor", sensor.Sensor, cg.Component)

CONF_BATTERY = "battery"
CONF_PM_25 = "pm_25"
CONF_PM_10 = "pm_10"
CONF_CARBON_DIOXIDE = "carbon_dioxide"
CONF_PACKET = "packet"

PC = new_pc(
    {
        CONF_TEMPERATURE: {
            CONF_TYPE_ID: qingping_pc_ns.class_("Temperature"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_TEMPERATURE,
            CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
            CONF_UNIT_OF_MEASUREMENT: UNIT_CELSIUS,
            CONF_ACCURACY_DECIMALS: 1,
        },
        CONF_HUMIDITY: {
            CONF_TYPE_ID: qingping_pc_ns.class_("Humidity"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_HUMIDITY,
            CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
            CONF_UNIT_OF_MEASUREMENT: UNIT_PERCENT,
            CONF_ACCURACY_DECIMALS: 1,
        },
        CONF_BATTERY_LEVEL: {
            CONF_TYPE_ID: qingping_pc_ns.class_("BatteryLevel"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_BATTERY,
            CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
            CONF_UNIT_OF_MEASUREMENT: UNIT_PERCENT,
            CONF_ACCURACY_DECIMALS: 0,
        },
        CONF_PRESSURE: {
            CONF_TYPE_ID: qingping_pc_ns.class_("Pressure"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_PRESSURE,
            CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
            CONF_UNIT_OF_MEASUREMENT: UNIT_HECTOPASCAL,
            CONF_ACCURACY_DECIMALS: 1,
        },
        CONF_ILLUMINANCE: {
            CONF_TYPE_ID: qingping_pc_ns.class_("Illuminance"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_ILLUMINANCE,
            CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
            CONF_UNIT_OF_MEASUREMENT: UNIT_LUX,
            CONF_ACCURACY_DECIMALS: 0,
        },
        CONF_PACKET: {
            CONF_TYPE_ID: qingping_pc_ns.class_("Packet"),
            CONF_ACCURACY_DECIMALS: 0,
        },
        CONF_PM25: {
            CONF_TYPE_ID: qingping_pc_ns.class_("PM25"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_PM25,
            CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
            CONF_UNIT_OF_MEASUREMENT: UNIT_MICROGRAMS_PER_CUBIC_METER,
            CONF_ACCURACY_DECIMALS: 0,
        },
        CONF_PM10: {
            CONF_TYPE_ID: qingping_pc_ns.class_("PM10"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_PM10,
            CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
            CONF_UNIT_OF_MEASUREMENT: UNIT_MICROGRAMS_PER_CUBIC_METER,
            CONF_ACCURACY_DECIMALS: 0,
        },
        CONF_CO2: {
            CONF_TYPE_ID: qingping_pc_ns.class_("CO2"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_CARBON_DIOXIDE,
            CONF_STATE_CLASS: STATE_CLASS_MEASUREMENT,
            CONF_UNIT_OF_MEASUREMENT: UNIT_PARTS_PER_MILLION,
            CONF_ACCURACY_DECIMALS: 0,
        },
        # aliases
        CONF_BATTERY: CONF_BATTERY_LEVEL,
        CONF_LIGHT: CONF_ILLUMINANCE,
        CONF_PM_25: CONF_PM25,
        CONF_PM_2_5: CONF_PM25,
        CONF_PM_10: CONF_PM10,
        CONF_CARBON_DIOXIDE: CONF_CO2,
    }
)


CONFIG_SCHEMA = PC.sensor_schema(QingpingSensor)


FINAL_VALIDATE_SCHEMA = qp_final_validate_config


async def to_code(config: dict):
    await PC.new_sensor(config)
