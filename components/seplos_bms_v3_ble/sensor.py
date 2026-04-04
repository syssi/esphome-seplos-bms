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
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_EMPTY,
    UNIT_HOUR,
    UNIT_PERCENT,
    UNIT_VOLT,
    UNIT_WATT,
    UNIT_WATT_HOURS,
)

from . import CONF_SEPLOS_BMS_V3_BLE_ID, SEPLOS_BMS_V3_BLE_COMPONENT_SCHEMA

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

# key: sensor_schema kwargs
SENSOR_DEFS = {
    CONF_TOTAL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CURRENT: {
        "unit_of_measurement": UNIT_AMPERE,
        "icon": "mdi:current-dc",
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_CURRENT,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_POWER: {
        "unit_of_measurement": UNIT_WATT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_POWER,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CHARGING_POWER: {
        "unit_of_measurement": UNIT_WATT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_POWER,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_DISCHARGING_POWER: {
        "unit_of_measurement": UNIT_WATT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_POWER,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CAPACITY_REMAINING: {
        "unit_of_measurement": UNIT_WATT_HOURS,
        "icon": "mdi:battery-50",
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_STATE_OF_CHARGE: {
        "unit_of_measurement": UNIT_PERCENT,
        "icon": "mdi:battery-50",
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CHARGING_CYCLES: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:battery-sync",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
    CONF_AVERAGE_CELL_TEMPERATURE: {
        "unit_of_measurement": UNIT_CELSIUS,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_PACK_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:package-variant",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_PROBLEM_CODE: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:alert-circle-outline",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CYCLE_CHARGE: {
        "unit_of_measurement": UNIT_WATT_HOURS,
        "icon": "mdi:battery-charging-100",
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
    CONF_CYCLE_CAPACITY: {
        "unit_of_measurement": UNIT_WATT_HOURS,
        "icon": "mdi:battery-50",
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
    CONF_RUNTIME: {
        "unit_of_measurement": UNIT_HOUR,
        "icon": "mdi:timer",
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
    CONF_STATE_OF_HEALTH: {
        "unit_of_measurement": UNIT_PERCENT,
        "icon": "mdi:battery-heart",
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_TOTAL_CAPACITY: {
        "unit_of_measurement": UNIT_WATT_HOURS,
        "icon": "mdi:battery-outline",
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_RATED_CAPACITY: {
        "unit_of_measurement": UNIT_WATT_HOURS,
        "icon": "mdi:battery-check",
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MIN_CELL_TEMPERATURE: {
        "unit_of_measurement": UNIT_CELSIUS,
        "icon": "mdi:thermometer-minus",
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_CELL_TEMPERATURE: {
        "unit_of_measurement": UNIT_CELSIUS,
        "icon": "mdi:thermometer-plus",
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MIN_TEMPERATURE_CELL: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:thermometer-minus",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_TEMPERATURE_CELL: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:thermometer-plus",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MIN_PACK_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": "mdi:battery-minus",
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_PACK_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": "mdi:battery-plus",
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MIN_PACK_VOLTAGE_ID: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:battery-minus",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_PACK_VOLTAGE_ID: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:battery-plus",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_SYSTEM_STATE_CODE: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:state-machine",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_VOLTAGE_EVENT_CODE: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:flash-alert",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_TEMPERATURE_EVENT_CODE: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:thermometer-alert",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CURRENT_EVENT_CODE: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:current-ac",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_DISCHARGE_CURRENT: {
        "unit_of_measurement": UNIT_AMPERE,
        "icon": "mdi:battery-arrow-down",
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_CURRENT,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_CHARGE_CURRENT: {
        "unit_of_measurement": UNIT_AMPERE,
        "icon": "mdi:battery-arrow-up",
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_CURRENT,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MIN_CELL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": "mdi:battery-minus-outline",
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_CELL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": "mdi:battery-plus-outline",
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MIN_VOLTAGE_CELL: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:battery-minus-outline",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_VOLTAGE_CELL: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:battery-plus-outline",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_DELTA_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": "mdi:battery-unknown",
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
}

CONFIG_SCHEMA = SEPLOS_BMS_V3_BLE_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(key): sensor.sensor_schema(**kwargs)
        for key, kwargs in SENSOR_DEFS.items()
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_SEPLOS_BMS_V3_BLE_ID])

    for key in SENSOR_DEFS:
        if key in config:
            conf = config[key]
            sens = await sensor.new_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_sensor")(sens))
