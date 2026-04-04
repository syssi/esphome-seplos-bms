import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_SEPLOS_BMS_V3_BLE_ID, SeplosBmsV3Ble

DEPENDENCIES = ["seplos_bms_v3_ble"]

CODEOWNERS = ["@syssi"]

CONF_CHARGING = "charging"
CONF_DISCHARGING = "discharging"
CONF_ONLINE_STATUS = "online_status"
CONF_VOLTAGE_PROTECTION = "voltage_protection"
CONF_TEMPERATURE_PROTECTION = "temperature_protection"
CONF_CURRENT_PROTECTION = "current_protection"
CONF_SYSTEM_FAULT = "system_fault"


# key: binary_sensor_schema kwargs
BINARY_SENSOR_DEFS = {
    CONF_CHARGING: {"icon": "mdi:battery-charging"},
    CONF_DISCHARGING: {"icon": "mdi:power-plug"},
    CONF_ONLINE_STATUS: {
        "icon": "mdi:heart-pulse",
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
    CONF_SYSTEM_FAULT: {
        "icon": "mdi:alert-circle",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SEPLOS_BMS_V3_BLE_ID): cv.use_id(SeplosBmsV3Ble),
    }
).extend(
    {
        cv.Optional(key): binary_sensor.binary_sensor_schema(**kwargs)
        for key, kwargs in BINARY_SENSOR_DEFS.items()
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_V3_BLE_ID])
    for key in BINARY_SENSOR_DEFS:
        if key in config:
            conf = config[key]
            sens = await binary_sensor.new_binary_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
