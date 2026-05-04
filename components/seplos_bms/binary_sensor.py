import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import DEVICE_CLASS_CONNECTIVITY, ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_SEPLOS_BMS_ID, SEPLOS_BMS_COMPONENT_SCHEMA

DEPENDENCIES = ["seplos_bms"]

CODEOWNERS = ["@syssi"]

CONF_ONLINE_STATUS = "online_status"
CONF_CHARGING = "charging"
CONF_DISCHARGING = "discharging"
CONF_VOLTAGE_PROTECTION = "voltage_protection"
CONF_TEMPERATURE_PROTECTION = "temperature_protection"
CONF_CURRENT_PROTECTION = "current_protection"
CONF_SOC_PROTECTION = "soc_protection"
CONF_BALANCING = "balancing"

# key: binary_sensor_schema kwargs
BINARY_SENSOR_DEFS = {
    CONF_ONLINE_STATUS: {
        "device_class": DEVICE_CLASS_CONNECTIVITY,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_CHARGING: {
        "icon": "mdi:battery-charging",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_DISCHARGING: {
        "icon": "mdi:power-plug",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_VOLTAGE_PROTECTION: {
        "icon": "mdi:flash-alert",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_TEMPERATURE_PROTECTION: {
        "icon": "mdi:thermometer-alert",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_CURRENT_PROTECTION: {
        "icon": "mdi:current-ac",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_SOC_PROTECTION: {
        "icon": "mdi:battery-alert",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_BALANCING: {
        "icon": "mdi:scale-balance",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
}

CONFIG_SCHEMA = SEPLOS_BMS_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(key): binary_sensor.binary_sensor_schema(**kwargs)
        for key, kwargs in BINARY_SENSOR_DEFS.items()
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_ID])
    for key in BINARY_SENSOR_DEFS:
        if key in config:
            conf = config[key]
            sens = await binary_sensor.new_binary_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
