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


BINARY_SENSORS = [
    CONF_CHARGING,
    CONF_DISCHARGING,
    CONF_ONLINE_STATUS,
    CONF_VOLTAGE_PROTECTION,
    CONF_TEMPERATURE_PROTECTION,
    CONF_CURRENT_PROTECTION,
    CONF_SYSTEM_FAULT,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SEPLOS_BMS_V3_BLE_ID): cv.use_id(SeplosBmsV3Ble),
        cv.Optional(CONF_CHARGING): binary_sensor.binary_sensor_schema(
            icon="mdi:battery-charging"
        ),
        cv.Optional(CONF_DISCHARGING): binary_sensor.binary_sensor_schema(
            icon="mdi:power-plug"
        ),
        cv.Optional(CONF_ONLINE_STATUS): binary_sensor.binary_sensor_schema(
            icon="mdi:heart-pulse",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_VOLTAGE_PROTECTION): binary_sensor.binary_sensor_schema(
            icon="mdi:flash-alert",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_TEMPERATURE_PROTECTION): binary_sensor.binary_sensor_schema(
            icon="mdi:thermometer-alert",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_CURRENT_PROTECTION): binary_sensor.binary_sensor_schema(
            icon="mdi:current-ac",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_SYSTEM_FAULT): binary_sensor.binary_sensor_schema(
            icon="mdi:alert-circle",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_V3_BLE_ID])
    for key in BINARY_SENSORS:
        if key in config:
            conf = config[key]
            sens = await binary_sensor.new_binary_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
