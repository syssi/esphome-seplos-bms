import esphome.codegen as cg
from esphome.components import seplos_bms_v3_ble
import esphome.config_validation as cv
from esphome.const import CONF_ID

AUTO_LOAD = ["sensor"]

CODEOWNERS = ["@syssi"]

MULTI_CONF = True

CONF_SEPLOS_BMS_V3_BLE_PACK_ID = "seplos_bms_v3_ble_pack_id"

DEFAULT_ADDRESS = 0x00

seplos_bms_v3_ble_pack_ns = cg.esphome_ns.namespace("seplos_bms_v3_ble_pack")
SeplosBmsV3BlePack = seplos_bms_v3_ble_pack_ns.class_(
    "SeplosBmsV3BlePack", cg.Component, seplos_bms_v3_ble.SeplosBmsV3BlePackBase
)

SEPLOS_BMS_V3_BLE_PACK_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SEPLOS_BMS_V3_BLE_PACK_ID): cv.use_id(SeplosBmsV3BlePack),
    }
)


def validate_address_unique(value):
    """Validate that each address is unique per parent BMS component."""
    from esphome.const import CONF_ADDRESS
    from esphome.core import CORE

    if not hasattr(CORE, "seplos_v3_ble_pack_addresses"):
        CORE.seplos_v3_ble_pack_addresses = {}

    parent_id = value[seplos_bms_v3_ble.CONF_SEPLOS_BMS_V3_BLE_ID]
    address = value[CONF_ADDRESS]

    if parent_id not in CORE.seplos_v3_ble_pack_addresses:
        CORE.seplos_v3_ble_pack_addresses[parent_id] = set()

    if address in CORE.seplos_v3_ble_pack_addresses[parent_id]:
        raise cv.Invalid(
            f"Address 0x{address:02X} is already used by another pack component for the same parent BMS"
        )

    CORE.seplos_v3_ble_pack_addresses[parent_id].add(address)
    return value


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SeplosBmsV3BlePack),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(seplos_bms_v3_ble.seplos_bms_v3_ble_device_schema(DEFAULT_ADDRESS)),
    validate_address_unique,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await seplos_bms_v3_ble.register_seplos_bms_v3_ble_device(var, config)
