import esphome.codegen as cg
from esphome.components import seplos_modbus
import esphome.config_validation as cv
from esphome.const import CONF_ID

AUTO_LOAD = ["seplos_modbus", "binary_sensor", "sensor", "text_sensor"]
CODEOWNERS = ["@syssi"]
MULTI_CONF = True

CONF_SEPLOS_BMS_ID = "seplos_bms_id"
CONF_ENABLE_FAKE_TRAFFIC = "enable_fake_traffic"

seplos_bms_ns = cg.esphome_ns.namespace("seplos_bms")
SeplosBms = seplos_bms_ns.class_(
    "SeplosBms", cg.PollingComponent, seplos_modbus.SeplosModbusDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SeplosBms),
            cv.Optional(CONF_ENABLE_FAKE_TRAFFIC, default=False): cv.boolean,
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(seplos_modbus.seplos_modbus_device_schema(0x00))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await seplos_modbus.register_seplos_modbus_device(var, config)

    cg.add(var.set_enable_fake_traffic(config[CONF_ENABLE_FAKE_TRAFFIC]))
