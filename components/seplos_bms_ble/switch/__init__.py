import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ID

from .. import (
    CONF_SEPLOS_BMS_BLE_ID,
    SEPLOS_BMS_BLE_COMPONENT_SCHEMA,
    seplos_bms_ble_ns,
)

DEPENDENCIES = ["seplos_bms_ble"]

CODEOWNERS = ["@syssi"]

SeplosBmsBle = seplos_bms_ble_ns.class_("SeplosBmsBle")

CONF_DISCHARGING = "discharging"
CONF_CHARGING = "charging"
CONF_CURRENT_LIMIT = "current_limit"
CONF_HEATING = "heating"

ICON_DISCHARGING = "mdi:battery-arrow-down-outline"
ICON_CHARGING = "mdi:battery-charging-100"
ICON_CURRENT_LIMIT = "mdi:current-ac"
ICON_HEATING = "mdi:radiator"

SWITCHES = {
    CONF_DISCHARGING: 0x01,  # Bit 0 - Discharge switch
    CONF_CHARGING: 0x02,  # Bit 1 - Charging switch
    CONF_CURRENT_LIMIT: 0x04,  # Bit 2 - Current limit switch
    CONF_HEATING: 0x08,  # Bit 3 - Heating switch
}

SeplosSwitch = seplos_bms_ble_ns.class_("SeplosSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = SEPLOS_BMS_BLE_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_DISCHARGING): switch.switch_schema(
            SeplosSwitch, icon=ICON_DISCHARGING
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_CHARGING): switch.switch_schema(
            SeplosSwitch, icon=ICON_CHARGING
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_CURRENT_LIMIT): switch.switch_schema(
            SeplosSwitch, icon=ICON_CURRENT_LIMIT
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_HEATING): switch.switch_schema(
            SeplosSwitch, icon=ICON_HEATING
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_BLE_ID])
    for key, payload in SWITCHES.items():
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await switch.register_switch(var, conf)
            cg.add(getattr(hub, f"set_{key}_switch")(var))
            cg.add(var.set_parent(hub))
            cg.add(var.set_holding_register(payload))
