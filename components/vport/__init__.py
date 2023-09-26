import esphome.codegen as cg
from esphome.components import ble_client, uart
import esphome.config_validation as cv
from esphome import core
from esphome.cpp_generator import MockObjClass
from esphome.const import CONF_ID

AUTO_LOAD = ["etl", "ble_client"]

IS_PLATFORM_COMPONENT = True

CONF_VPORT_ID = "vport_id"
CONF_VPORT_IO_ID = "vport_io_id"
CONF_PERSISTENT_CONNECTION = "persistent_connection"
CONF_DISABLE_SCAN = "disable_scan"


vport_ns = cg.esphome_ns.namespace("vport")
VPort = vport_ns.class_("VPort")

VPORT_CLIENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_VPORT_ID): cv.use_id(VPort),
    }
)


def vport_schema(
    vport_class: MockObjClass,
    io_class: MockObjClass,
    default_update_interval=None,
):
    if not vport_class.inherits_from(VPort):
        raise cv.Invalid("Not a VPort Component")
    schema = cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(vport_class),
            cv.GenerateID(CONF_VPORT_IO_ID): cv.declare_id(io_class),
        }
    )
    if default_update_interval is None:
        schema = schema.extend(cv.COMPONENT_SCHEMA)
    else:
        schema = schema.extend(cv.polling_component_schema(default_update_interval))
    return schema


def vport_ble_schema(
    vport_class: MockObjClass,
    io_class: MockObjClass,
    default_update_interval="60s",
):
    return (
        vport_schema(vport_class, io_class, default_update_interval)
        .extend(ble_client.BLE_CLIENT_SCHEMA)
        .extend(
            {
                cv.Optional(CONF_PERSISTENT_CONNECTION, default=False): cv.boolean,
                cv.Optional(CONF_DISABLE_SCAN, default=False): cv.boolean,
            }
        )
    )


def vport_uart_schema(
    vport_class: MockObjClass,
    io_class: MockObjClass,
    default_update_interval=None,
):
    return vport_schema(vport_class, io_class, default_update_interval).extend(
        uart.UART_DEVICE_SCHEMA
    )


def _find_by_id(platform: str, find_id) -> core.ID:
    for _, item in enumerate(core.CORE.config[platform]):
        item_id: core.ID = item["id"]
        if str(item_id.id) == str(find_id):
            return item_id
    return None


def vport_find_by_id(find_id):
    return _find_by_id("vport", find_id)


def vport_find(config):
    return vport_find_by_id(config[CONF_VPORT_ID])


async def vport_get_var(config):
    return await cg.get_variable(config[CONF_VPORT_ID])


async def setup_vport_ble(config):
    vio = cg.new_Pvariable(config[CONF_VPORT_IO_ID])
    await ble_client.register_ble_node(vio, config)

    var = cg.new_Pvariable(config[CONF_ID], vio)
    await cg.register_component(var, config)
    cg.add(var.set_persistent_connection(config[CONF_PERSISTENT_CONNECTION]))
    # FIXME reimplement and add again
    # cg.add(var.set_disable_scan(config[CONF_DISABLE_SCAN]))
    return var


async def setup_vport_uart(config):
    urt = await cg.get_variable(config[uart.CONF_UART_ID])
    vio = cg.new_Pvariable(config[CONF_VPORT_IO_ID], urt)
    var = cg.new_Pvariable(config[CONF_ID], vio)
    await cg.register_component(var, config)
    return var


async def to_code(config):
    if "uart" in core.CORE.config:
        cg.add_build_flag("-DUSE_VPORT_UART")
    if "esp32_ble_tracker" in core.CORE.config:
        cg.add_build_flag("-DUSE_VPORT_BLE")
