from esphome.components.ble_client import switch
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
    CONF_CURRENT,
    CONF_DEVICE_CLASS,
    CONF_FREQUENCY,
    CONF_ID,
    CONF_NUMBER,
    CONF_POWER_FACTOR,
    CONF_REACTIVE_POWER,
    CONF_VOLTAGE,
    CONF_ENERGY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_POWER_FACTOR,
    DEVICE_CLASS_VOLTAGE,
    ESP_PLATFORM_ESP8266,
    ICON_CURRENT_AC,
    ICON_EMPTY,
    LAST_RESET_TYPE_AUTO,
    STATE_CLASS_MEASUREMENT,
    UNIT_EMPTY,
    UNIT_HERTZ,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_VOLT_AMPS,
    UNIT_VOLT_AMPS_REACTIVE,
    UNIT_WATT,
)

ESP_PLATFORMS = [ESP_PLATFORM_ESP8266]

CODEOWNERS = ["@dentra"]

AUTO_LOAD = ["switch", "sensor", "binary_sensor"]
DEPENDENCIES = ["uart"]

UNIT_KWATT_HOURS = "kWh"
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
    binary_sensor.BINARY_SENSOR_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(ZMAi90v1),
            cv.Optional(CONF_SWITCH_PIN, default=DEFAULT_SWITCH_PIN): valid_switch_pin,
            cv.Optional(CONF_BUTTON_PIN, default=DEFAULT_BUTTON_PIN): valid_button_pin,
            cv.Optional(CONF_BUTTON): binary_sensor.BINARY_SENSOR_SCHEMA.extend(
                {
                    cv.Optional(
                        CONF_DEVICE_CLASS, default=DEVICE_CLASS_EMPTY
                    ): binary_sensor.device_class,
                    cv.Optional(CONF_BIND_TO_SWITCH, default=True): cv.boolean,
                }
            ),
            cv.Optional(CONF_ENERGY): sensor.sensor_schema(
                UNIT_KWATT_HOURS,
                ICON_EMPTY,
                2,
                DEVICE_CLASS_ENERGY,
                STATE_CLASS_MEASUREMENT,
                LAST_RESET_TYPE_AUTO,
            ),
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                UNIT_VOLT, ICON_EMPTY, 2, DEVICE_CLASS_VOLTAGE
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                UNIT_AMPERE,
                ICON_EMPTY,
                3,
                DEVICE_CLASS_CURRENT,
                STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ACTIVE_POWER): sensor.sensor_schema(
                UNIT_WATT, ICON_EMPTY, 2, DEVICE_CLASS_POWER, STATE_CLASS_MEASUREMENT
            ),
            cv.Optional(CONF_APPARENT_POWER): sensor.sensor_schema(
                UNIT_VOLT_AMPS,
                ICON_EMPTY,
                2,
                DEVICE_CLASS_POWER,
                STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_REACTIVE_POWER): sensor.sensor_schema(
                UNIT_VOLT_AMPS_REACTIVE,
                ICON_EMPTY,
                2,
                DEVICE_CLASS_POWER,
                STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FREQUENCY): sensor.sensor_schema(
                UNIT_HERTZ,
                ICON_CURRENT_AC,
                2,
                DEVICE_CLASS_EMPTY,
                STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_POWER_FACTOR): sensor.sensor_schema(
                UNIT_EMPTY,
                ICON_EMPTY,
                1,
                DEVICE_CLASS_POWER_FACTOR,
                STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


def valid_uart(conf):
    baud_rate = 9600
    if conf[uart.CONF_BAUD_RATE] != baud_rate:
        raise cv.Invalid(
            f"Component {component_name} required baud_rate {baud_rate} for the uart bus"
        )
    rx_pin = 3
    if conf[uart.CONF_RX_PIN] != rx_pin:
        raise cv.Invalid(
            f"Component {component_name} required rx_pin GPIO0{rx_pin} for the uart bus"
        )
    tx_pin = 1
    if conf[uart.CONF_TX_PIN] != tx_pin:
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
        cv.declare_id(binary_sensor.ClickTrigger)(id + "trigger"),
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
        cv.declare_id(switch.ToggleAction)(id + "action"),
        cg.TemplateArguments(),
        switch_,
    )
    cg.add(automat.add_actions([action]))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    switch_pin = await cg.gpio_pin_expression(config[CONF_SWITCH_PIN])
    cg.add(var.set_switch_pin(switch_pin))

    button_pin = await cg.gpio_pin_expression(config[CONF_BUTTON_PIN])
    cg.add(var.set_button_pin(button_pin))

    if CONF_BUTTON in config:
        conf = config[CONF_BUTTON]
        button = cg.new_Pvariable(conf[CONF_ID])
        await binary_sensor.register_binary_sensor(button, conf)
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
