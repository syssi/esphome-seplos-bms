import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID

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

BINARY_SENSORS = [
    CONF_CHARGING,
    CONF_DISCHARGING,
    CONF_LIMITING_CURRENT,
    CONF_ONLINE_STATUS,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SEPLOS_BMS_BLE_ID): cv.use_id(SeplosBmsBle),
        cv.Optional(CONF_CHARGING): binary_sensor.binary_sensor_schema(
            icon=ICON_CHARGING
        ),
        cv.Optional(CONF_DISCHARGING): binary_sensor.binary_sensor_schema(
            icon=ICON_DISCHARGING
        ),
        cv.Optional(CONF_LIMITING_CURRENT): binary_sensor.binary_sensor_schema(
            icon=ICON_LIMITING_CURRENT
        ),
        cv.Optional(CONF_ONLINE_STATUS): binary_sensor.binary_sensor_schema(
            icon=ICON_ONLINE_STATUS
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_BLE_ID])
    for key in BINARY_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await binary_sensor.register_binary_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
