import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID

from . import CONF_SEPLOS_BMS_V3_BLE_PACK_ID, SeplosBmsV3BlePack

DEPENDENCIES = ["seplos_bms_v3_ble_pack"]

CODEOWNERS = ["@syssi"]

CONF_PACK_STATUS = "data"

TEXT_SENSORS = [
    CONF_PACK_STATUS,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SEPLOS_BMS_V3_BLE_PACK_ID): cv.use_id(SeplosBmsV3BlePack),
        cv.Optional(CONF_PACK_STATUS): text_sensor.text_sensor_schema(
            icon="mdi:battery-check"
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_V3_BLE_PACK_ID])
    for key in TEXT_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await text_sensor.register_text_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
