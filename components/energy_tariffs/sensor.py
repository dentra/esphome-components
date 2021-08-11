import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.core import TimePeriod
from esphome.components import sensor, time
from esphome import automation
from esphome.const import (
    CONF_ID,
    CONF_TIME_ID,
    CONF_TOTAL,
    CONF_TRIGGER_ID,
    CONF_SERVICE,
    DEVICE_CLASS_ENERGY,
)

CODEOWNERS = ["@dentra"]

DEPENDENCIES = ["time"]

UNIT_KWATT_HOURS = "kWh"

# CONF_CURRENT_TARIFF = 'current_tariff'
CONF_TARIFFS = "tariffs"
CONF_TIME = "time"
CONF_TIME_OFFSET = "time_offset"
CONF_TIME_OFFSET_SERVICE = "time_offset_service"
CONF_ON_TARIFF = "on_tariff"
CONF_ON_BEFORE_TARIFF = "on_before_tariff"

ICON_CURRENT_TARIFF = "mdi:theme-light-dark"
ICON_TARIFF = ICON_CURRENT_TARIFF
ICON_TARIFF_DAY = "mdi:weather-sunny"
ICON_TARIFF_NIGHT = "mdi:weather-night"

energy_tariffs_ns = cg.esphome_ns.namespace("energy_tariffs")

EnergyTariffs = energy_tariffs_ns.class_("EnergyTariffs", cg.Component)

EnergyTariff = energy_tariffs_ns.class_("EnergyTariff", sensor.Sensor)
EnergyTariffPtr = EnergyTariff.operator("ptr")

TariffChangeTrigger = energy_tariffs_ns.class_(
    "TariffChangeTrigger", automation.Trigger.template(sensor.SensorPtr)
)
BeforeTariffChangeTrigger = energy_tariffs_ns.class_(
    "BeforeTariffChangeTrigger", automation.Trigger.template()
)

# TariffSetAction = energy_statistics_ns.class_('TariffSetAction', automation.Action)
# TariffIsCondition = energy_statistics_ns.class_('TariffIsCondition', automation.Condition)

TIME_PERIOD_ERROR = "Time {} should be format DD:MM"


def time_period(value):
    """Validate and transform time with format HH:MM."""
    if not isinstance(value, str):
        raise cv.Invalid(TIME_PERIOD_ERROR.format(value))

    try:
        parsed = [int(x) for x in value.split(":")]
    except ValueError:
        raise cv.Invalid(TIME_PERIOD_ERROR.format(value))

    if len(parsed) == 2:
        hour, minute = parsed
    else:
        raise cv.Invalid(TIME_PERIOD_ERROR.format(value))

    return TimePeriod(hours=hour, minutes=minute)


def validate_tariff_time(value):
    value = cv.string_strict(value)
    parts = value.split("-")
    if len(parts) != 2:
        raise cv.Invalid("Time period should be HH:MM-HH:MM format")
    time_period(parts[0])
    time_period(parts[1])
    return value


def validate_tariffs(config):
    if CONF_TARIFFS not in config:
        return config

    tariffs = config.get(CONF_TARIFFS, [])
    count = len(tariffs)
    if count == 0:
        return config

    for tariff in tariffs:
        if CONF_TIME in tariff:
            count -= 1

    if count == 0:
        raise cv.Invalid(
            "One tariff (usually peak) is required to be default (without time)"
        )

    if count > 1:
        raise cv.Invalid(
            "Only one tariff (usually peak) must be default (without time)"
        )

    return config


TARIFF_SCHEMA = sensor.sensor_schema(
    UNIT_KWATT_HOURS, ICON_TARIFF, 2, DEVICE_CLASS_ENERGY
).extend(
    {
        cv.GenerateID(): cv.declare_id(EnergyTariff),
        cv.Optional(CONF_TIME): cv.All(
            cv.ensure_list(validate_tariff_time), cv.Length(min=1, max=3)
        ),
        cv.Optional(CONF_SERVICE): cv.valid_name,
    }
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(EnergyTariffs),
            cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
            cv.Required(CONF_TOTAL): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_TIME_OFFSET): cv.int_,
            cv.Optional(CONF_TIME_OFFSET_SERVICE): cv.valid_name,
            cv.Optional(CONF_TARIFFS): cv.All(
                cv.ensure_list(TARIFF_SCHEMA), cv.Length(min=1, max=4)
            ),
            cv.Optional(CONF_ON_TARIFF): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(TariffChangeTrigger),
                }
            ),
            cv.Optional(CONF_ON_BEFORE_TARIFF): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
                        BeforeTariffChangeTrigger
                    ),
                }
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    validate_tariffs,
)


async def setup_sensor(config, key, setter):
    if key not in config:
        return None
    var = await sensor.new_sensor(config[key])
    cg.add(setter(var))
    return var


async def setup_input(config, key, setter):
    if key not in config:
        return None
    var = await cg.get_variable(config[key])
    cg.add(setter(var))
    return var


# code generation entry point
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    await setup_input(config, CONF_TIME_ID, var.set_time)

    # inputs
    await setup_input(config, CONF_TOTAL, var.set_total)

    if CONF_TIME_OFFSET in config:
        cg.add(var.set_time_offset(config[CONF_TIME_OFFSET]))
    if CONF_TIME_OFFSET_SERVICE in config:
        cg.add(var.set_time_offset_service(config[CONF_TIME_OFFSET_SERVICE]))

    # exposed sensors
    for conf in config.get(CONF_TARIFFS, []):
        sens = cg.new_Pvariable(conf[CONF_ID])
        await sensor.register_sensor(sens, conf)
        for tm in conf.get(CONF_TIME, []):
            parts = tm.split("-")
            t = [time_period(parts[0]), time_period(parts[1])]
            cg.add(sens.add_time(t[0].total_minutes, t[1].total_minutes))
        cg.add(var.add_tariff(sens))
        if CONF_SERVICE in conf:
            cg.add(sens.set_service(conf[CONF_SERVICE]))

    for conf in config.get(CONF_ON_TARIFF, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(sensor.SensorPtr, "x")], conf)

    for conf in config.get(CONF_ON_BEFORE_TARIFF, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)


# @automation.register_action('tariff.set', TariffSetAction,
#    cv.Schema({
#        cv.Required(CONF_ID): cv.use_id(EnergyTariff),
#        cv.Required(CONF_STATE): cv.templatable(cv.float_),
#    }))
# def tariff_set_offset_to_code(config, action_id, template_arg, args):
#    paren = await cg.get_variable(config[CONF_ID])
#    var = cg.new_Pvariable(action_id, template_arg, paren)
#    template_ = await cg.templatable(config[CONF_STATE], args, float)
#    cg.add(var.set_state(template_))
#    await var

# @automation.register_condition('tariff.is', TariffIsCondition, cv.Schema({
#    cv.Required(CONF_ID): cv.use_id(EnergyStatistics),
#    cv.Required(CONF_STATE): cv.templatable(EnergyTariffPtr),
# }))
# def tariff_is_to_code(config, condition_id, template_arg, args):
#    paren = await cg.get_variable(config[CONF_ID])
#    var = cg.new_Pvariable(condition_id, template_arg, paren)
#    templ = await cg.templatable(config[CONF_STATE], args, cg.std_string)
#    cg.add(var.set_state(templ))
#    await var
