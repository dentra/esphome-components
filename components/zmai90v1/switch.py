import logging
from esphome import core, pins
import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome import automation
from esphome.components import binary_sensor, sensor, uart, switch
from esphome.const import (
    CONF_ACTIVE_POWER,
    CONF_APPARENT_POWER,
    CONF_BAUD_RATE,
    CONF_CURRENT,
    CONF_FREQUENCY,
    CONF_ID,
    CONF_NUMBER,
    CONF_POWER_FACTOR,
    CONF_REACTIVE_POWER,
    CONF_RESTORE_MODE,
    CONF_RX_PIN,
    CONF_TX_PIN,
    CONF_VOLTAGE,
    CONF_ENERGY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_POWER_FACTOR,
    DEVICE_CLASS_VOLTAGE,
    PLATFORM_ESP8266,
    ICON_CURRENT_AC,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_HERTZ,
    UNIT_KILOWATT_HOURS,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_VOLT_AMPS,
    UNIT_VOLT_AMPS_REACTIVE,
    UNIT_WATT,
)

ESP_PLATFORMS = [PLATFORM_ESP8266]

CODEOWNERS = ["@dentra"]

AUTO_LOAD = ["switch", "sensor", "binary_sensor"]
DEPENDENCIES = ["uart"]

CONF_SWITCH_PIN = "switch_pin"
CONF_BUTTON_PIN = "button_pin"
CONF_BUTTON = "button"
CONF_BIND_TO_SWITCH = "bind_to_switch"

DEFAULT_SWITCH_PIN = 12
DEFAULT_BUTTON_PIN = 13

_LOGGER = logging.getLogger(__name__)

component_name = "zmai90v1"

zmai90v1_ns = cg.esphome_ns.namespace(component_name)
ZMAi90v1 = zmai90v1_ns.class_(
    "ZMAi90v1", cg.PollingComponent, switch.Switch, uart.UARTDevice
)
ZMAi90v1RestoreMode = zmai90v1_ns.enum("ZMAi90v1RestoreMode")

RESTORE_MODES = {
    "ALWAYS_ON": ZMAi90v1RestoreMode.RESTORE_MODE_ALWAYS_ON,
    "ALWAYS_OFF": ZMAi90v1RestoreMode.RESTORE_MODE_ALWAYS_OFF,
    "RESTORE_DEFAULT_ON": ZMAi90v1RestoreMode.RESTORE_MODE_RESTORE_DEFAULT_ON,
    "RESTORE_DEFAULT_OFF": ZMAi90v1RestoreMode.RESTORE_MODE_RESTORE_DEFAULT_OFF,
}


def valid_switch_pin(value):
    value = pins.internal_gpio_output_pin_schema(value)
    num = value[CONF_NUMBER]
    if num != DEFAULT_SWITCH_PIN:
        _LOGGER.warning(f"Usually switch pin is GPIO{DEFAULT_SWITCH_PIN}")
    return value


def valid_button_pin(value):
    value = pins.gpio_input_pin_schema(value)
    num = value[CONF_NUMBER]
    if num != DEFAULT_BUTTON_PIN:
        _LOGGER.warning(f"Usually button pin is GPIO{DEFAULT_BUTTON_PIN}")
    return value


CONFIG_SCHEMA = (
    switch.switch_schema(ZMAi90v1)
    .extend(
        {
            cv.Optional(CONF_SWITCH_PIN, default=DEFAULT_SWITCH_PIN): valid_switch_pin,
            cv.Optional(CONF_BUTTON_PIN, default=DEFAULT_BUTTON_PIN): valid_button_pin,
            cv.Optional(CONF_BUTTON): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_EMPTY
            ).extend(
                {
                    cv.Optional(CONF_BIND_TO_SWITCH, default=True): cv.boolean,
                }
            ),
            cv.Optional(CONF_ENERGY): sensor.sensor_schema(
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_TOTAL_INCREASING,
            ),
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ACTIVE_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_APPARENT_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT_AMPS,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_REACTIVE_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT_AMPS_REACTIVE,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FREQUENCY): sensor.sensor_schema(
                unit_of_measurement=UNIT_HERTZ,
                icon=ICON_CURRENT_AC,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_POWER_FACTOR): sensor.sensor_schema(
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_POWER_FACTOR,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_RESTORE_MODE, default="ALWAYS_ON"): cv.enum(
                RESTORE_MODES, upper=True, space="_"
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


def valid_uart(conf):
    baud_rate = 9600
    if conf[CONF_BAUD_RATE] != baud_rate:
        raise cv.Invalid(
            f"Component {component_name} required baud_rate {baud_rate} for the uart bus"
        )
    rx_pin = 3
    if conf[CONF_RX_PIN][CONF_NUMBER] != rx_pin:
        raise cv.Invalid(
            f"Component {component_name} required rx_pin GPIO0{rx_pin} for the uart bus"
        )
    tx_pin = 1
    if conf[CONF_TX_PIN][CONF_NUMBER] != tx_pin:
        raise cv.Invalid(
            f"Component {component_name} required tx_pin GPIO0{tx_pin} for the uart bus"
        )
    parity = "EVEN"
    if conf[uart.CONF_PARITY] != parity:
        raise cv.Invalid(
            f"Component {component_name} required parity {parity} for the uart bus"
        )
    stop_bits = 1
    if conf[uart.CONF_STOP_BITS] != stop_bits:
        raise cv.Invalid(
            f"Component {component_name} required stop_bits {stop_bits} for the uart bus"
        )
    data_bits = 8
    if conf[uart.CONF_DATA_BITS] != data_bits:
        raise cv.Invalid(
            f"Component {component_name} required data_bits {data_bits} for the uart bus"
        )
    return conf


FINAL_VALIDATE_SCHEMA = cv.All(
    cv.Schema(
        {cv.Required(uart.CONF_UART_ID): fv.id_declaration_match_schema(valid_uart)},
        extra=cv.ALLOW_EXTRA,
    ),
)


def add_button_trigger_(id_: core.ID, switch_, button_):
    id = id_.id + "_"
    trigger = cg.new_Pvariable(
        cv.declare_id(binary_sensor.ClickTrigger)(id + "clicktrigger"),
        button_,
        50,
        350,
    )
    automat = cg.new_Pvariable(
        cv.declare_id(automation.Automation)(id + "automation"),
        cg.TemplateArguments(),
        trigger,
    )
    action = cg.new_Pvariable(
        cv.declare_id(switch.ToggleAction)(id + "toggleaction"),
        cg.TemplateArguments(),
        switch_,
    )
    cg.add(automat.add_actions([action]))


async def to_code(config):
    """Code generation entry point"""
    var = cg.new_Pvariable(config[CONF_ID])
    await switch.register_switch(var, config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_restore_mode(config[CONF_RESTORE_MODE]))

    switch_pin = await cg.gpio_pin_expression(config[CONF_SWITCH_PIN])
    cg.add(var.set_switch_pin(switch_pin))

    button_pin = await cg.gpio_pin_expression(config[CONF_BUTTON_PIN])
    cg.add(var.set_button_pin(button_pin))

    if CONF_BUTTON in config:
        conf = config[CONF_BUTTON]
        button = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_button(button))
        if conf[CONF_BIND_TO_SWITCH]:
            add_button_trigger_(conf[CONF_ID], var, button)

    if CONF_ENERGY in config:
        conf = config[CONF_ENERGY]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_energy(sens))
    if CONF_VOLTAGE in config:
        conf = config[CONF_VOLTAGE]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_voltage(sens))
    if CONF_CURRENT in config:
        conf = config[CONF_CURRENT]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_current(sens))
    if CONF_ACTIVE_POWER in config:
        conf = config[CONF_ACTIVE_POWER]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_active_power(sens))
    if CONF_APPARENT_POWER in config:
        conf = config[CONF_APPARENT_POWER]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_apparent_power(sens))
    if CONF_REACTIVE_POWER in config:
        conf = config[CONF_REACTIVE_POWER]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_reactive_power(sens))
    if CONF_FREQUENCY in config:
        conf = config[CONF_FREQUENCY]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_frequency(sens))
    if CONF_POWER_FACTOR in config:
        conf = config[CONF_POWER_FACTOR]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_power_factor(sens))
