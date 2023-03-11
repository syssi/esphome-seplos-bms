import esphome.codegen as cg
from esphome.components import seplos_modbus
import esphome.config_validation as cv
from esphome.const import CONF_ID

AUTO_LOAD = ["seplos_modbus", "binary_sensor", "sensor", "text_sensor"]
CODEOWNERS = ["@syssi"]
MULTI_CONF = True

CONF_SEPLOS_BMS_ID = "seplos_bms_id"

DEFAULT_PROTOCOL_VERSION = 0x20
DEFAULT_ADDRESS = 0x00

seplos_bms_ns = cg.esphome_ns.namespace("seplos_bms")
SeplosBms = seplos_bms_ns.class_(
    "SeplosBms", cg.PollingComponent, seplos_modbus.SeplosModbusDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SeplosBms),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(
        seplos_modbus.seplos_modbus_device_schema(
            DEFAULT_PROTOCOL_VERSION, DEFAULT_ADDRESS
        )
    )
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await seplos_modbus.register_seplos_modbus_device(var, config)
