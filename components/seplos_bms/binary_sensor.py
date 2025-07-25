import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, DEVICE_CLASS_CONNECTIVITY, ENTITY_CATEGORY_DIAGNOSTIC

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

BINARY_SENSORS = [
    CONF_ONLINE_STATUS,
    CONF_CHARGING,
    CONF_DISCHARGING,
    CONF_VOLTAGE_PROTECTION,
    CONF_TEMPERATURE_PROTECTION,
    CONF_CURRENT_PROTECTION,
    CONF_SOC_PROTECTION,
]

CONFIG_SCHEMA = SEPLOS_BMS_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_ONLINE_STATUS): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_CONNECTIVITY,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_CHARGING): binary_sensor.binary_sensor_schema(
            icon="mdi:battery-charging",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_DISCHARGING): binary_sensor.binary_sensor_schema(
            icon="mdi:power-plug",
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
        cv.Optional(CONF_SOC_PROTECTION): binary_sensor.binary_sensor_schema(
            icon="mdi:battery-alert",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_ID])
    for key in BINARY_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await binary_sensor.register_binary_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
