import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_CURRENT,
    CONF_POWER,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ICON_EMPTY,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_EMPTY,
    UNIT_HOUR,
    UNIT_PERCENT,
    UNIT_VOLT,
    UNIT_WATT,
    UNIT_WATT_HOURS,
)

from . import CONF_SEPLOS_BMS_V3_BLE_ID, SeplosBmsV3Ble

DEPENDENCIES = ["seplos_bms_v3_ble"]

CODEOWNERS = ["@syssi"]

CONF_TOTAL_VOLTAGE = "total_voltage"
CONF_CHARGING_POWER = "charging_power"
CONF_DISCHARGING_POWER = "discharging_power"
CONF_STATE_OF_CHARGE = "state_of_charge"
CONF_CHARGING_CYCLES = "charging_cycles"
CONF_MIN_CELL_VOLTAGE = "min_cell_voltage"
CONF_MAX_CELL_VOLTAGE = "max_cell_voltage"
CONF_MIN_VOLTAGE_CELL = "min_voltage_cell"
CONF_MAX_VOLTAGE_CELL = "max_voltage_cell"
CONF_DELTA_VOLTAGE = "delta_voltage"
CONF_AVERAGE_CELL_TEMPERATURE = "average_cell_temperature"
CONF_PACK_COUNT = "pack_count"
CONF_PROBLEM_CODE = "problem_code"
CONF_CYCLE_CHARGE = "cycle_charge"
CONF_CYCLE_CAPACITY = "cycle_capacity"
CONF_RUNTIME = "runtime"
CONF_STATE_OF_HEALTH = "state_of_health"
CONF_CAPACITY_REMAINING = "capacity_remaining"
CONF_TOTAL_CAPACITY = "total_capacity"
CONF_RATED_CAPACITY = "rated_capacity"
CONF_MIN_CELL_TEMPERATURE = "min_cell_temperature"
CONF_MAX_CELL_TEMPERATURE = "max_cell_temperature"
CONF_MIN_TEMPERATURE_CELL = "min_temperature_cell"
CONF_MAX_TEMPERATURE_CELL = "max_temperature_cell"
CONF_MIN_PACK_VOLTAGE = "min_pack_voltage"
CONF_MAX_PACK_VOLTAGE = "max_pack_voltage"
CONF_MIN_PACK_VOLTAGE_ID = "min_pack_voltage_id"
CONF_MAX_PACK_VOLTAGE_ID = "max_pack_voltage_id"
CONF_SYSTEM_STATE_CODE = "system_state_code"
CONF_VOLTAGE_EVENT_CODE = "voltage_event_code"
CONF_TEMPERATURE_EVENT_CODE = "temperature_event_code"
CONF_CURRENT_EVENT_CODE = "current_event_code"
CONF_MAX_DISCHARGE_CURRENT = "max_discharge_current"
CONF_MAX_CHARGE_CURRENT = "max_charge_current"

SENSORS = [
    CONF_TOTAL_VOLTAGE,
    CONF_CURRENT,
    CONF_POWER,
    CONF_CHARGING_POWER,
    CONF_DISCHARGING_POWER,
    CONF_CAPACITY_REMAINING,
    CONF_STATE_OF_CHARGE,
    CONF_CHARGING_CYCLES,
    CONF_AVERAGE_CELL_TEMPERATURE,
    CONF_PACK_COUNT,
    CONF_PROBLEM_CODE,
    CONF_CYCLE_CHARGE,
    CONF_CYCLE_CAPACITY,
    CONF_RUNTIME,
    CONF_STATE_OF_HEALTH,
    CONF_TOTAL_CAPACITY,
    CONF_RATED_CAPACITY,
    CONF_MIN_CELL_TEMPERATURE,
    CONF_MAX_CELL_TEMPERATURE,
    CONF_MIN_TEMPERATURE_CELL,
    CONF_MAX_TEMPERATURE_CELL,
    CONF_MIN_PACK_VOLTAGE,
    CONF_MAX_PACK_VOLTAGE,
    CONF_MIN_PACK_VOLTAGE_ID,
    CONF_MAX_PACK_VOLTAGE_ID,
    CONF_SYSTEM_STATE_CODE,
    CONF_VOLTAGE_EVENT_CODE,
    CONF_TEMPERATURE_EVENT_CODE,
    CONF_CURRENT_EVENT_CODE,
    CONF_MAX_DISCHARGE_CURRENT,
    CONF_MAX_CHARGE_CURRENT,
    CONF_MIN_CELL_VOLTAGE,
    CONF_MAX_CELL_VOLTAGE,
    CONF_MIN_VOLTAGE_CELL,
    CONF_MAX_VOLTAGE_CELL,
    CONF_DELTA_VOLTAGE,
]


def sensor_schema(unit, icon, accuracy_decimals=1, device_class=DEVICE_CLASS_EMPTY):
    return sensor.sensor_schema(
        unit_of_measurement=unit,
        icon=icon,
        accuracy_decimals=accuracy_decimals,
        device_class=device_class,
        state_class=STATE_CLASS_MEASUREMENT,
    )


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SEPLOS_BMS_V3_BLE_ID): cv.use_id(SeplosBmsV3Ble),
        cv.Optional(CONF_TOTAL_VOLTAGE): sensor_schema(
            UNIT_VOLT, ICON_EMPTY, 2, DEVICE_CLASS_VOLTAGE
        ),
        cv.Optional(CONF_CURRENT): sensor_schema(
            UNIT_AMPERE, "mdi:current-dc", 1, DEVICE_CLASS_CURRENT
        ),
        cv.Optional(CONF_POWER): sensor_schema(
            UNIT_WATT, ICON_EMPTY, 1, DEVICE_CLASS_POWER
        ),
        cv.Optional(CONF_CHARGING_POWER): sensor_schema(
            UNIT_WATT, ICON_EMPTY, 1, DEVICE_CLASS_POWER
        ),
        cv.Optional(CONF_DISCHARGING_POWER): sensor_schema(
            UNIT_WATT, ICON_EMPTY, 1, DEVICE_CLASS_POWER
        ),
        cv.Optional(CONF_CAPACITY_REMAINING): sensor_schema(
            UNIT_WATT_HOURS, "mdi:battery-50", 1
        ),
        cv.Optional(CONF_STATE_OF_CHARGE): sensor_schema(
            UNIT_PERCENT, "mdi:battery-50", 1
        ),
        cv.Optional(CONF_CHARGING_CYCLES): sensor_schema(
            UNIT_EMPTY, "mdi:battery-sync", 0
        ),
        cv.Optional(CONF_AVERAGE_CELL_TEMPERATURE): sensor_schema(
            UNIT_CELSIUS, ICON_EMPTY, 1, DEVICE_CLASS_TEMPERATURE
        ),
        cv.Optional(CONF_PACK_COUNT): sensor_schema(
            UNIT_EMPTY, "mdi:package-variant", 0
        ),
        cv.Optional(CONF_PROBLEM_CODE): sensor_schema(
            UNIT_EMPTY, "mdi:alert-circle-outline", 0
        ),
        cv.Optional(CONF_CYCLE_CHARGE): sensor_schema(
            UNIT_WATT_HOURS, "mdi:battery-charging-100", 2
        ),
        cv.Optional(CONF_CYCLE_CAPACITY): sensor_schema(
            UNIT_WATT_HOURS, "mdi:battery-50", 2
        ),
        cv.Optional(CONF_RUNTIME): sensor_schema(UNIT_HOUR, "mdi:timer", 1),
        cv.Optional(CONF_STATE_OF_HEALTH): sensor_schema(
            UNIT_PERCENT, "mdi:battery-heart", 1
        ),
        cv.Optional(CONF_TOTAL_CAPACITY): sensor_schema(
            UNIT_WATT_HOURS, "mdi:battery-outline", 1
        ),
        cv.Optional(CONF_RATED_CAPACITY): sensor_schema(
            UNIT_WATT_HOURS, "mdi:battery-check", 1
        ),
        cv.Optional(CONF_MIN_CELL_TEMPERATURE): sensor_schema(
            UNIT_CELSIUS, "mdi:thermometer-minus", 1, DEVICE_CLASS_TEMPERATURE
        ),
        cv.Optional(CONF_MAX_CELL_TEMPERATURE): sensor_schema(
            UNIT_CELSIUS, "mdi:thermometer-plus", 1, DEVICE_CLASS_TEMPERATURE
        ),
        cv.Optional(CONF_MIN_TEMPERATURE_CELL): sensor_schema(
            UNIT_EMPTY, "mdi:thermometer-minus", 0
        ),
        cv.Optional(CONF_MAX_TEMPERATURE_CELL): sensor_schema(
            UNIT_EMPTY, "mdi:thermometer-plus", 0
        ),
        cv.Optional(CONF_MIN_PACK_VOLTAGE): sensor_schema(
            UNIT_VOLT, "mdi:battery-minus", 2, DEVICE_CLASS_VOLTAGE
        ),
        cv.Optional(CONF_MAX_PACK_VOLTAGE): sensor_schema(
            UNIT_VOLT, "mdi:battery-plus", 2, DEVICE_CLASS_VOLTAGE
        ),
        cv.Optional(CONF_MIN_PACK_VOLTAGE_ID): sensor_schema(
            UNIT_EMPTY, "mdi:battery-minus", 0
        ),
        cv.Optional(CONF_MAX_PACK_VOLTAGE_ID): sensor_schema(
            UNIT_EMPTY, "mdi:battery-plus", 0
        ),
        cv.Optional(CONF_SYSTEM_STATE_CODE): sensor_schema(
            UNIT_EMPTY, "mdi:state-machine", 0
        ),
        cv.Optional(CONF_VOLTAGE_EVENT_CODE): sensor_schema(
            UNIT_EMPTY, "mdi:flash-alert", 0
        ),
        cv.Optional(CONF_TEMPERATURE_EVENT_CODE): sensor_schema(
            UNIT_EMPTY, "mdi:thermometer-alert", 0
        ),
        cv.Optional(CONF_CURRENT_EVENT_CODE): sensor_schema(
            UNIT_EMPTY, "mdi:current-ac", 0
        ),
        cv.Optional(CONF_MAX_DISCHARGE_CURRENT): sensor_schema(
            UNIT_AMPERE, "mdi:battery-arrow-down", 1, DEVICE_CLASS_CURRENT
        ),
        cv.Optional(CONF_MAX_CHARGE_CURRENT): sensor_schema(
            UNIT_AMPERE, "mdi:battery-arrow-up", 1, DEVICE_CLASS_CURRENT
        ),
        cv.Optional(CONF_MIN_CELL_VOLTAGE): sensor_schema(
            UNIT_VOLT, "mdi:battery-minus-outline", 3, DEVICE_CLASS_VOLTAGE
        ),
        cv.Optional(CONF_MAX_CELL_VOLTAGE): sensor_schema(
            UNIT_VOLT, "mdi:battery-plus-outline", 3, DEVICE_CLASS_VOLTAGE
        ),
        cv.Optional(CONF_MIN_VOLTAGE_CELL): sensor_schema(
            UNIT_EMPTY, "mdi:battery-minus-outline", 0
        ),
        cv.Optional(CONF_MAX_VOLTAGE_CELL): sensor_schema(
            UNIT_EMPTY, "mdi:battery-plus-outline", 0
        ),
        cv.Optional(CONF_DELTA_VOLTAGE): sensor_schema(
            UNIT_VOLT, "mdi:battery-unknown", 3, DEVICE_CLASS_VOLTAGE
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_V3_BLE_ID])

    for key in SENSORS:
        if key in config:
            conf = config[key]
            sens = await sensor.new_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_sensor")(sens))
