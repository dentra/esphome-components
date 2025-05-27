from typing import Any

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    binary_sensor,
    button,
    climate,
    fan,
    number,
    select,
    sensor,
    switch,
    text_sensor,
)
from esphome.const import (
    CONF_ACCURACY_DECIMALS,
    CONF_DEVICE_CLASS,
    CONF_ENTITY_CATEGORY,
    CONF_ICON,
    CONF_ID,
    CONF_STATE_CLASS,
    CONF_TYPE,
    CONF_TYPE_ID,
    CONF_UNIT_OF_MEASUREMENT,
)

CONF_COMPONENT_CLASS = "component_class"


class PC:
    def __init__(
        self,
        parent_class: cg.MockObjClass,
        parent_id: str,
        pc_cfg: dict[str, str | dict[str, Any]],
        component_type: str = "",
    ) -> None:
        self.parent_class = parent_class
        self.parent_id = parent_id
        self.pc_cfg = pc_cfg
        self.component_type = component_type

    def _pc_schema_update_config(
        self, config: dict[str, Any], value_validators: dict[str, Any]
    ):
        default_value_validators = {
            CONF_ICON: cv.icon,
            CONF_ENTITY_CATEGORY: cv.entity_category,
        }
        if value_validators:
            default_value_validators.update(value_validators)
        value_validators = default_value_validators

        pc_nfo = self.pc_cfg[config[CONF_TYPE]]
        if not isinstance(pc_nfo, dict):
            config[CONF_TYPE] = str(pc_nfo)
            pc_nfo = self.pc_cfg[config[CONF_TYPE]]

        for key, val in pc_nfo.items():
            if key in config or key == CONF_TYPE_ID or str(key).startswith("_"):
                continue
            if key in value_validators:
                val = value_validators[key](val)
            config[key] = val

        return config

    def component_schema(
        self,
        schema: cv.Schema,
        ext_schema: dict = None,
        validators: list = None,
        value_validators: dict[str, Any] = None,
    ):
        schema = schema.extend(
            {cv.GenerateID(self.parent_id): cv.use_id(self.parent_class)}
        )

        if self.pc_cfg:
            schema = schema.extend(
                {cv.Required(CONF_TYPE): cv.one_of(*self.pc_cfg.keys(), lower=True)}
            )

        if ext_schema:
            schema = schema.extend(ext_schema)

        schema = schema.extend(cv.COMPONENT_SCHEMA)

        if validators:
            schema = cv.All(schema, *validators)

        if self.pc_cfg:

            def validator(config):
                return self._pc_schema_update_config(config, value_validators)

            schema = cv.All(schema, validator)

        return schema

    def get_info(self, config: dict) -> dict[str, Any]:
        pc_typ = self.get_type(config)
        if not pc_typ:
            return dict()
        return self.pc_cfg.get(pc_typ, dict())

    def get_info_prop(self, config: dict, prop: str, default_value) -> Any:
        return self.get_info(config).get(prop, default_value)

    def get_type(self, config: dict, default_value=""):
        return config.get(CONF_TYPE, default_value)

    def get_type_component(self, config: dict):
        return self.get_info(config).get(CONF_COMPONENT_CLASS, None)

    def get_type_class(self, config: dict):
        return self.get_info(config).get(CONF_TYPE_ID, None)

    async def new_component(self, config: dict, ctor, **kwargs):
        parent = await cg.get_variable(config[self.parent_id])

        if cls := self.get_type_component(config):
            # hacky inject CONF_COMPONENT_CLASS into CONF_ID
            config[CONF_ID].type = cls

        if type_class := self.get_type_class(config):
            var = await ctor(config, cg.TemplateArguments(type_class), parent, **kwargs)
        else:
            var = await ctor(config, parent, **kwargs)

        await cg.register_component(var, config)

        cg.add(var.set_component_source(self._get_component_source(config)))
        return var

    def _get_component_source(self, config: dict):
        prefix = self.parent_id.replace("_id", "")
        if self.component_type:
            prefix = f"{prefix}.{self.component_type}"
        if pc_typ := self.get_type(config):
            return f"{prefix}[type={pc_typ}]"
        return prefix

    def button_schema(
        self,
        button_class: cg.MockObjClass,
        ext_schema: dict = None,
        validators: list = None,
    ):
        return self.component_schema(
            button.button_schema(button_class),
            ext_schema=ext_schema,
            validators=validators,
            value_validators={
                CONF_DEVICE_CLASS: button.validate_device_class,
            },
        )

    async def new_button(self, config: dict, **kwargs):
        return await self.new_component(config, button.new_button, **kwargs)

    def sensor_schema(self, sensor_class: cg.MockObjClass):
        return self.component_schema(
            sensor.sensor_schema(sensor_class),
            value_validators={
                CONF_DEVICE_CLASS: sensor.validate_device_class,
                CONF_ENTITY_CATEGORY: sensor.sensor_entity_category,
                CONF_STATE_CLASS: sensor.validate_state_class,
                CONF_UNIT_OF_MEASUREMENT: sensor.validate_unit_of_measurement,
                CONF_ACCURACY_DECIMALS: sensor.validate_accuracy_decimals,
            },
        )

    async def new_sensor(self, config: dict):
        return await self.new_component(config, sensor.new_sensor)

    def binary_sensor_schema(self, binary_sensor_class: cg.MockObjClass):
        return self.component_schema(
            binary_sensor.binary_sensor_schema(binary_sensor_class),
            value_validators={
                CONF_DEVICE_CLASS: binary_sensor.validate_device_class,
            },
        )

    async def new_binary_sensor(self, config: dict):
        return await self.new_component(config, binary_sensor.new_binary_sensor)

    def text_sensor_schema(self, text_sensor_class: cg.MockObjClass):
        return self.component_schema(
            text_sensor.text_sensor_schema(text_sensor_class),
            value_validators={
                CONF_DEVICE_CLASS: text_sensor.validate_device_class,
            },
        )

    async def new_text_sensor(self, config: dict):
        return await self.new_component(config, text_sensor.new_text_sensor)

    def number_schema(self, number_class: cg.MockObjClass, schema_ext: dict = None):
        return self.component_schema(
            number.number_schema(number_class),
            ext_schema=schema_ext,
            value_validators={
                CONF_DEVICE_CLASS: number.validate_device_class,
                CONF_UNIT_OF_MEASUREMENT: number.validate_unit_of_measurement,
            },
        )

    async def new_number(
        self, config: dict, min_value: float, max_value: float, step: float
    ):
        return await self.new_component(
            config,
            number.new_number,
            min_value=min_value,
            max_value=max_value,
            step=step,
        )

    def select_schema(self, select_class: cg.MockObjClass, schema_ext: dict = None):
        return self.component_schema(
            select.select_schema(select_class), ext_schema=schema_ext
        )

    async def new_select(self, config: dict, options: list[str] = None):
        async def _select_new_select(config, *args):
            var = cg.new_Pvariable(config[CONF_ID], *args)
            await select.register_select(var, config, options=options or [])
            return var

        # select.new_select do not pass additional args to ctor, so use own impl
        return await self.new_component(config, _select_new_select)

    def switch_schema(
        self,
        switch_class: cg.MockObjClass,
        schema_ext: dict = None,
        validators: list = None,
    ):
        return self.component_schema(
            switch.switch_schema(switch_class, block_inverted=True),
            ext_schema=schema_ext,
            validators=validators,
            value_validators={
                CONF_DEVICE_CLASS: switch.validate_device_class,
            },
        )

    async def new_switch(self, config: dict):
        return await self.new_component(config, switch.new_switch)

    def climate_schema(
        self,
        climate_class: cg.MockObjClass,
        *,
        entity_category: str = cv.UNDEFINED,
        icon: str = cv.UNDEFINED,
        ext_schema: dict = None,
    ):
        return self.component_schema(
            climate.climate_schema(
                climate_class, entity_category=entity_category, icon=icon
            ),
            ext_schema=ext_schema,
        )

    async def new_climate(self, config: dict):
        return await self.new_component(config, climate.new_climate)

    def fan_schema(
        self,
        fan_class: cg.MockObjClass,
        *,
        entity_category: str = cv.UNDEFINED,
        icon: str = cv.UNDEFINED,
        default_restore_mode: str = cv.UNDEFINED,
        ext_schema: dict = None,
    ):
        return self.component_schema(
            fan.fan_schema(
                fan_class,
                entity_category=entity_category,
                icon=icon,
                default_restore_mode=default_restore_mode,
            ),
            ext_schema=ext_schema,
        )

    async def new_fan(self, config: dict):
        return await self.new_component(config, fan.new_fan)
