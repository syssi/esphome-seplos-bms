import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID

from . import CONF_SEPLOS_BMS_ID, SeplosBms

DEPENDENCIES = ["seplos_bms"]

CODEOWNERS = ["@syssi"]

CONF_FAN_RUNNING = "fan_running"

BINARY_SENSORS = [
    CONF_FAN_RUNNING,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SEPLOS_BMS_ID): cv.use_id(SeplosBms),
        cv.Optional(CONF_FAN_RUNNING): binary_sensor.binary_sensor_schema(
            icon="mdi:fan"
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
