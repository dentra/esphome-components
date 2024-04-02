import esphome.codegen as cg
from esphome.components import binary_sensor
from esphome.const import (
    CONF_DEVICE_CLASS,
    CONF_ENTITY_CATEGORY,
    CONF_LIGHT,
    CONF_MOTION,
    CONF_TYPE_ID,
    DEVICE_CLASS_DOOR,
    DEVICE_CLASS_OPENING,
    DEVICE_CLASS_PROBLEM,
    DEVICE_CLASS_WINDOW,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

from .. import new_pc, qingping_ns, qingping_pc_ns, qp_final_validate_config

QingpingBinarySensor = qingping_ns.class_(
    "QingpingBinarySensor", binary_sensor.BinarySensor, cg.Component
)
CONF_DOOR = "door"
CONF_WINDOW = "window"
CONF_OPENING = "opening"
CONF_DOOR_PROBLEM = "door_problem"
CONF_WINDOW_PROBLEM = "window_problem"
CONF_OPENING_PROBLEM = "opening_problem"

PC = new_pc(
    {
        CONF_DOOR: {
            CONF_TYPE_ID: qingping_pc_ns.class_("Door"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_DOOR,
        },
        CONF_DOOR_PROBLEM: {
            CONF_TYPE_ID: qingping_pc_ns.class_("DoorProblem"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_PROBLEM,
            CONF_ENTITY_CATEGORY: ENTITY_CATEGORY_DIAGNOSTIC,
        },
        CONF_WINDOW: {
            CONF_TYPE_ID: qingping_pc_ns.class_("Door"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_WINDOW,
        },
        CONF_OPENING: {
            CONF_TYPE_ID: qingping_pc_ns.class_("Door"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_OPENING,
        },
        CONF_MOTION: {
            CONF_TYPE_ID: qingping_pc_ns.class_("Motion"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_DOOR,
        },
        CONF_LIGHT: {
            CONF_TYPE_ID: qingping_pc_ns.class_("Light"),
            CONF_DEVICE_CLASS: DEVICE_CLASS_DOOR,
        },
        # aliases
        CONF_WINDOW_PROBLEM: CONF_DOOR_PROBLEM,
        CONF_OPENING_PROBLEM: CONF_DOOR_PROBLEM,
    }
)


CONFIG_SCHEMA = PC.binary_sensor_schema(QingpingBinarySensor)

FINAL_VALIDATE_SCHEMA = qp_final_validate_config


async def to_code(config: dict):
    await PC.new_binary_sensor(config)
