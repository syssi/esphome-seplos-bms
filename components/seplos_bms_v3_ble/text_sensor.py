import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID

from . import CONF_SEPLOS_BMS_V3_BLE_ID, SeplosBmsV3Ble

DEPENDENCIES = ["seplos_bms_v3_ble"]

CODEOWNERS = ["@syssi"]

CONF_PROBLEM = "problem"
CONF_FACTORY_NAME = "factory_name"
CONF_DEVICE_NAME = "device_name"
CONF_FIRMWARE_VERSION = "firmware_version"
CONF_BMS_SERIAL_NUMBER = "bms_serial_number"
CONF_PACK_SERIAL_NUMBER = "pack_serial_number"


TEXT_SENSORS = [
    CONF_PROBLEM,
    CONF_FACTORY_NAME,
    CONF_DEVICE_NAME,
    CONF_FIRMWARE_VERSION,
    CONF_BMS_SERIAL_NUMBER,
    CONF_PACK_SERIAL_NUMBER,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SEPLOS_BMS_V3_BLE_ID): cv.use_id(SeplosBmsV3Ble),
        cv.Optional(CONF_PROBLEM): text_sensor.text_sensor_schema(
            icon="mdi:alert-circle-outline"
        ),
        cv.Optional(CONF_FACTORY_NAME): text_sensor.text_sensor_schema(
            icon="mdi:factory"
        ),
        cv.Optional(CONF_DEVICE_NAME): text_sensor.text_sensor_schema(icon="mdi:chip"),
        cv.Optional(CONF_FIRMWARE_VERSION): text_sensor.text_sensor_schema(
            icon="mdi:information-outline"
        ),
        cv.Optional(CONF_BMS_SERIAL_NUMBER): text_sensor.text_sensor_schema(
            icon="mdi:barcode"
        ),
        cv.Optional(CONF_PACK_SERIAL_NUMBER): text_sensor.text_sensor_schema(
            icon="mdi:battery-outline"
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_V3_BLE_ID])
    for key in TEXT_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await text_sensor.register_text_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
