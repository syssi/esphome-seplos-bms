import esphome.codegen as cg
from esphome.components import ble_client
import esphome.config_validation as cv
from esphome.const import CONF_ADDRESS, CONF_ID

CODEOWNERS = ["@syssi"]

AUTO_LOAD = ["binary_sensor", "sensor", "text_sensor"]

MULTI_CONF = True

CONF_SEPLOS_BMS_V3_BLE_ID = "seplos_bms_v3_ble_id"

seplos_bms_v3_ble_ns = cg.esphome_ns.namespace("seplos_bms_v3_ble")
SeplosBmsV3Ble = seplos_bms_v3_ble_ns.class_(
    "SeplosBmsV3Ble", ble_client.BLEClientNode, cg.PollingComponent
)
SeplosBmsV3BlePackBase = seplos_bms_v3_ble_ns.class_("SeplosBmsV3BlePack")

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SeplosBmsV3Ble),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.polling_component_schema("10s"))
)


def seplos_bms_v3_ble_device_schema(default_address):
    schema = {
        cv.GenerateID(CONF_SEPLOS_BMS_V3_BLE_ID): cv.use_id(SeplosBmsV3Ble),
        cv.Optional(CONF_ADDRESS, default=default_address): cv.hex_uint8_t,
    }
    return cv.Schema(schema)


async def register_seplos_bms_v3_ble_device(var, config):
    parent = await cg.get_variable(config[CONF_SEPLOS_BMS_V3_BLE_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(parent.register_pack_component(var))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
