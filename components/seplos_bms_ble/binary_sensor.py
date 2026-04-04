import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_SEPLOS_BMS_BLE_ID, SeplosBmsBle

DEPENDENCIES = ["seplos_bms_ble"]

CODEOWNERS = ["@syssi"]

CONF_CHARGING = "charging"
CONF_DISCHARGING = "discharging"
CONF_LIMITING_CURRENT = "limiting_current"
CONF_ONLINE_STATUS = "online_status"

ICON_CHARGING = "mdi:battery-charging"
ICON_DISCHARGING = "mdi:power-plug"
ICON_LIMITING_CURRENT = "mdi:car-speed-limiter"
ICON_ONLINE_STATUS = "mdi:heart-pulse"

# key: binary_sensor_schema kwargs
BINARY_SENSOR_DEFS = {
    CONF_CHARGING: {"icon": ICON_CHARGING},
    CONF_DISCHARGING: {"icon": ICON_DISCHARGING},
    CONF_LIMITING_CURRENT: {
        "icon": ICON_LIMITING_CURRENT,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_ONLINE_STATUS: {
        "icon": ICON_ONLINE_STATUS,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SEPLOS_BMS_BLE_ID): cv.use_id(SeplosBmsBle),
    }
).extend(
    {
        cv.Optional(key): binary_sensor.binary_sensor_schema(**kwargs)
        for key, kwargs in BINARY_SENSOR_DEFS.items()
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_BLE_ID])
    for key in BINARY_SENSOR_DEFS:
        if key in config:
            conf = config[key]
            sens = await binary_sensor.new_binary_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
