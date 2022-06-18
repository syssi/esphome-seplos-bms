import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import seplos_modbus
from esphome.const import CONF_ID

AUTO_LOAD = ["seplos_modbus"]
CODEOWNERS = ["@syssi"]
MULTI_CONF = True

CONF_SEPLOS_BMS_ID = "seplos_bms_id"

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
    .extend(cv.polling_component_schema("2s"))
    .extend(seplos_modbus.seplos_modbus_device_schema(0x00))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await seplos_modbus.register_seplos_modbus_device(var, config)
