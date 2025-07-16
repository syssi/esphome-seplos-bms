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
CONF_PACK_CELL_VOLTAGE_1 = "pack_cell_voltage_1"
CONF_PACK_CELL_VOLTAGE_2 = "pack_cell_voltage_2"
CONF_PACK_CELL_VOLTAGE_3 = "pack_cell_voltage_3"
CONF_PACK_CELL_VOLTAGE_4 = "pack_cell_voltage_4"
CONF_PACK_CELL_VOLTAGE_5 = "pack_cell_voltage_5"
CONF_PACK_CELL_VOLTAGE_6 = "pack_cell_voltage_6"
CONF_PACK_CELL_VOLTAGE_7 = "pack_cell_voltage_7"
CONF_PACK_CELL_VOLTAGE_8 = "pack_cell_voltage_8"
CONF_PACK_CELL_VOLTAGE_9 = "pack_cell_voltage_9"
CONF_PACK_CELL_VOLTAGE_10 = "pack_cell_voltage_10"
CONF_PACK_CELL_VOLTAGE_11 = "pack_cell_voltage_11"
CONF_PACK_CELL_VOLTAGE_12 = "pack_cell_voltage_12"
CONF_PACK_CELL_VOLTAGE_13 = "pack_cell_voltage_13"
CONF_PACK_CELL_VOLTAGE_14 = "pack_cell_voltage_14"
CONF_PACK_CELL_VOLTAGE_15 = "pack_cell_voltage_15"
CONF_PACK_CELL_VOLTAGE_16 = "pack_cell_voltage_16"
CONF_PACK_TEMPERATURE_1 = "pack_temperature_1"
CONF_PACK_TEMPERATURE_2 = "pack_temperature_2"
CONF_PACK_TEMPERATURE_3 = "pack_temperature_3"
CONF_PACK_TEMPERATURE_4 = "pack_temperature_4"
CONF_AMBIENT_TEMPERATURE = "ambient_temperature"  # PIB register 1118
CONF_MOSFET_TEMPERATURE = "mosfet_temperature"  # PIB register 1119


TYPES = [
    CONF_PACK_VOLTAGE,
    CONF_PACK_CURRENT,
    CONF_PACK_BATTERY_LEVEL,
    CONF_PACK_CYCLE,
    CONF_PACK_CELL_VOLTAGE_1,
    CONF_PACK_CELL_VOLTAGE_2,
    CONF_PACK_CELL_VOLTAGE_3,
    CONF_PACK_CELL_VOLTAGE_4,
    CONF_PACK_CELL_VOLTAGE_5,
    CONF_PACK_CELL_VOLTAGE_6,
    CONF_PACK_CELL_VOLTAGE_7,
    CONF_PACK_CELL_VOLTAGE_8,
    CONF_PACK_CELL_VOLTAGE_9,
    CONF_PACK_CELL_VOLTAGE_10,
    CONF_PACK_CELL_VOLTAGE_11,
    CONF_PACK_CELL_VOLTAGE_12,
    CONF_PACK_CELL_VOLTAGE_13,
    CONF_PACK_CELL_VOLTAGE_14,
    CONF_PACK_CELL_VOLTAGE_15,
    CONF_PACK_CELL_VOLTAGE_16,
    CONF_PACK_TEMPERATURE_1,
    CONF_PACK_TEMPERATURE_2,
    CONF_PACK_TEMPERATURE_3,
    CONF_PACK_TEMPERATURE_4,
    CONF_AMBIENT_TEMPERATURE,
    CONF_MOSFET_TEMPERATURE,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_PACK_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CURRENT): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_BATTERY_LEVEL): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_BATTERY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CYCLE): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_COUNTER,
            accuracy_decimals=0,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_1): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_2): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_3): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_4): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_5): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_6): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_7): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_8): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_9): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_10): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_11): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_12): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_13): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_14): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_15): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_CELL_VOLTAGE_16): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_TEMPERATURE_1): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_TEMPERATURE_2): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_TEMPERATURE_3): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PACK_TEMPERATURE_4): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_AMBIENT_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_MOSFET_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
).extend(SEPLOS_BMS_V3_BLE_PACK_COMPONENT_SCHEMA)


async def to_code(config):
    from . import CONF_SEPLOS_BMS_V3_BLE_PACK_ID

    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_V3_BLE_PACK_ID])
    for key in TYPES:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[cv.CONF_ID])
            await sensor.register_sensor(sens, conf)

            # Use generic index-based setters for cell voltages and temperatures
            if key.startswith("pack_cell_voltage_"):
                index = (
                    int(key.rsplit("_", maxsplit=1)[-1]) - 1
                )  # Convert to 0-based index
                cg.add(hub.set_pack_cell_voltage_sensor(index, sens))
            elif key.startswith("pack_temperature_"):
                index = (
                    int(key.rsplit("_", maxsplit=1)[-1]) - 1
                )  # Convert to 0-based index
                cg.add(hub.set_pack_temperature_sensor(index, sens))
            else:
                # Use individual setter methods for other sensors
                cg.add(getattr(hub, f"set_{key}_sensor")(sens))
