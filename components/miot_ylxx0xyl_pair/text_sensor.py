from esphome.const import CONF_BINDKEY, CONF_ID, CONF_NAME, CONF_VERSION
from esphome.components import esp32_ble_tracker
from esphome.components.esp32_ble_tracker import CONF_ESP32_BLE_ID
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import miot, text_sensor, ble_client

CODEOWNERS = ["@dentra"]
AUTO_LOAD = ["miot", "text_sensor", "ble_client"]

CONF_TOKEN = "token"
CONF_BEACONKEY = "beakonkey"
miot_ylxx0xyl_pair_ns = cg.esphome_ns.namespace("miot_ylxx0xyl_pair")
MiotYLxx0xYLPair = miot_ylxx0xyl_pair_ns.class_(
    "MiotYLxx0xYLPair", miot.MiotComponent, text_sensor.TextSensor
)

CONFIG_SCHEMA = (
    text_sensor.TEXT_SENSOR_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(MiotYLxx0xYLPair),
            cv.Optional(CONF_BEACONKEY): text_sensor.TEXT_SENSOR_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
                }
            ),
            cv.Optional(CONF_VERSION): text_sensor.TEXT_SENSOR_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
                }
            ),
        }
    ).extend(miot.MIOT_BLE_DEVICE_SCHEMA)
    # .extend(ble_client.BLE_CLIENT_SCHEMA)
)


async def to_code(config):
    var = await miot.new_text_sensor_device(config)
    ble_tracker = await cg.get_variable(config[CONF_ESP32_BLE_ID])
    cg.add(var.set_ble_tracker(ble_tracker))
    # await esp32_ble_tracker.register_client(var, config)

    # await ble_client.register_ble_node(var, config)
    # cli = await cg.get_variable(config[ble_client.CONF_BLE_CLIENT_ID])
    # cg.add(var.set_ble_client(cli))

    if CONF_BEACONKEY in config:
        sens = cg.new_Pvariable(config[CONF_BEACONKEY][CONF_ID])
        await text_sensor.register_text_sensor(sens, config[CONF_BEACONKEY])
        cg.add(var.set_bindkey(sens))
    if CONF_VERSION in config:
        sens = cg.new_Pvariable(config[CONF_VERSION][CONF_ID])
        await text_sensor.register_text_sensor(sens, config[CONF_VERSION])
        cg.add(var.set_version(sens))
