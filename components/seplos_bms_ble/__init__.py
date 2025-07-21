import esphome.codegen as cg
from esphome.components import ble_client
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@syssi"]

AUTO_LOAD = ["binary_sensor", "sensor", "switch", "text_sensor"]

MULTI_CONF = True

CONF_SEPLOS_BMS_BLE_ID = "seplos_bms_ble_id"

seplos_bms_ble_ns = cg.esphome_ns.namespace("seplos_bms_ble")
SeplosBmsBle = seplos_bms_ble_ns.class_(
    "SeplosBmsBle", ble_client.BLEClientNode, cg.PollingComponent
)

SEPLOS_BMS_BLE_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SEPLOS_BMS_BLE_ID): cv.use_id(SeplosBmsBle),
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SeplosBmsBle),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.polling_component_schema("10s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
