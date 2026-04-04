import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ICON_COUNTER,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_EMPTY,
    UNIT_PERCENT,
    UNIT_VOLT,
)

from . import SEPLOS_BMS_V3_BLE_PACK_COMPONENT_SCHEMA

UNIT_AMPERE_HOURS = "Ah"

DEPENDENCIES = ["seplos_bms_v3_ble_pack"]
CODEOWNERS = ["@syssi"]

CONF_PACK_VOLTAGE = "pack_voltage"
CONF_PACK_CURRENT = "pack_current"
CONF_PACK_BATTERY_LEVEL = "pack_battery_level"
CONF_PACK_CYCLE = "pack_cycle"
CONF_AMBIENT_TEMPERATURE = "ambient_temperature"  # PIB register 1118
CONF_MOSFET_TEMPERATURE = "mosfet_temperature"  # PIB register 1119

PACK_CELLS = [f"pack_cell_voltage_{i}" for i in range(1, 17)]
PACK_TEMPERATURES = [f"pack_temperature_{i}" for i in range(1, 5)]

# key: sensor_schema kwargs
SENSOR_DEFS = {
    CONF_PACK_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_PACK_CURRENT: {
        "unit_of_measurement": UNIT_AMPERE,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_CURRENT,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_PACK_BATTERY_LEVEL: {
        "unit_of_measurement": UNIT_PERCENT,
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_BATTERY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_PACK_CYCLE: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_COUNTER,
        "accuracy_decimals": 0,
    },
    CONF_AMBIENT_TEMPERATURE: {
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MOSFET_TEMPERATURE: {
        "unit_of_measurement": UNIT_CELSIUS,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
}

_PACK_CELL_VOLTAGE_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=3,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
)
_PACK_TEMPERATURE_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=2,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(key): sensor.sensor_schema(**kwargs)
            for key, kwargs in SENSOR_DEFS.items()
        }
    )
    .extend({cv.Optional(key): _PACK_CELL_VOLTAGE_SCHEMA for key in PACK_CELLS})
    .extend({cv.Optional(key): _PACK_TEMPERATURE_SCHEMA for key in PACK_TEMPERATURES})
    .extend(SEPLOS_BMS_V3_BLE_PACK_COMPONENT_SCHEMA)
)


async def to_code(config):
    from . import CONF_SEPLOS_BMS_V3_BLE_PACK_ID

    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_V3_BLE_PACK_ID])
    for key in SENSOR_DEFS:
        if key in config:
            conf = config[key]
            sens = await sensor.new_sensor(conf)

            # Use individual setter methods for other sensors
            cg.add(getattr(hub, f"set_{key}_sensor")(sens))
    for key in PACK_CELLS:
        if key in config:
            conf = config[key]
            sens = await sensor.new_sensor(conf)

            # Use generic index-based setters for cell voltages
            index = int(key.rsplit("_", maxsplit=1)[-1]) - 1  # Convert to 0-based index
            cg.add(hub.set_pack_cell_voltage_sensor(index, sens))
    for key in PACK_TEMPERATURES:
        if key in config:
            conf = config[key]
            sens = await sensor.new_sensor(conf)

            # Use generic index-based setters for temperatures
            index = int(key.rsplit("_", maxsplit=1)[-1]) - 1  # Convert to 0-based index
            cg.add(hub.set_pack_temperature_sensor(index, sens))
