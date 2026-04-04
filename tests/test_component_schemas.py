"""Schema structure tests for seplos_bms ESPHome component modules."""

import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import components.seplos_bms as hub  # noqa: E402
from components.seplos_bms import binary_sensor, sensor, text_sensor  # noqa: E402
import components.seplos_bms_ble as hub_ble  # noqa: E402
from components.seplos_bms_ble import (  # noqa: E402
    binary_sensor as ble_binary_sensor,
    sensor as ble_sensor,
    switch as ble_switch,  # noqa: E402
    text_sensor as ble_text_sensor,
)
import components.seplos_bms_v3_ble as hub_v3  # noqa: E402
from components.seplos_bms_v3_ble import (  # noqa: E402
    binary_sensor as v3_binary_sensor,
    sensor as v3_sensor,
    text_sensor as v3_text_sensor,
)


class TestHubConstants:
    def test_conf_ids_defined(self):
        assert hub.CONF_SEPLOS_BMS_ID == "seplos_bms_id"
        assert hub_ble.CONF_SEPLOS_BMS_BLE_ID == "seplos_bms_ble_id"
        assert hub_v3.CONF_SEPLOS_BMS_V3_BLE_ID == "seplos_bms_v3_ble_id"


class TestSeplosBmsSensorLists:
    def test_cells_count(self):
        assert len(sensor.CELLS) == 16

    def test_cells_naming(self):
        assert sensor.CELLS[0] == "cell_voltage_1"
        assert sensor.CELLS[15] == "cell_voltage_16"
        for i, key in enumerate(sensor.CELLS, 1):
            assert key == f"cell_voltage_{i}"

    def test_temperatures_count(self):
        assert len(sensor.TEMPERATURES) == 6

    def test_sensor_defs_completeness(self):
        assert "total_voltage" in sensor.SENSOR_DEFS
        assert "state_of_charge" in sensor.SENSOR_DEFS
        assert len(sensor.SENSOR_DEFS) == 18

    def test_no_cell_keys_in_sensor_defs(self):
        for key in sensor.SENSOR_DEFS:
            assert key not in sensor.CELLS
            assert key not in sensor.TEMPERATURES


class TestSeplosBmsBinarySensorConstants:
    def test_binary_sensor_defs_dict(self):
        assert binary_sensor.CONF_ONLINE_STATUS in binary_sensor.BINARY_SENSOR_DEFS
        assert len(binary_sensor.BINARY_SENSOR_DEFS) == 1


class TestSeplosBmsTextSensorConstants:
    def test_text_sensors_list(self):
        assert text_sensor.CONF_ERRORS in text_sensor.TEXT_SENSORS
        assert len(text_sensor.TEXT_SENSORS) == 1


class TestSeplosBmsBleSensorLists:
    def test_cells_count(self):
        assert len(ble_sensor.CELLS) == 24

    def test_cells_naming(self):
        assert ble_sensor.CELLS[0] == "cell_voltage_1"
        assert ble_sensor.CELLS[23] == "cell_voltage_24"

    def test_temperatures_count(self):
        assert len(ble_sensor.TEMPERATURES) == 8

    def test_sensor_defs_completeness(self):
        assert "total_voltage" in ble_sensor.SENSOR_DEFS
        assert len(ble_sensor.SENSOR_DEFS) == 32

    def test_no_cell_keys_in_sensor_defs(self):
        for key in ble_sensor.SENSOR_DEFS:
            assert key not in ble_sensor.CELLS
            assert key not in ble_sensor.TEMPERATURES


class TestSeplosBmsBleBinarySensorConstants:
    def test_binary_sensor_defs_dict(self):
        assert ble_binary_sensor.CONF_CHARGING in ble_binary_sensor.BINARY_SENSOR_DEFS
        assert (
            ble_binary_sensor.CONF_DISCHARGING in ble_binary_sensor.BINARY_SENSOR_DEFS
        )
        assert len(ble_binary_sensor.BINARY_SENSOR_DEFS) == 4


class TestSeplosBmsBleTextSensorConstants:
    def test_text_sensors_list(self):
        assert ble_text_sensor.CONF_SOFTWARE_VERSION in ble_text_sensor.TEXT_SENSORS
        assert ble_text_sensor.CONF_DEVICE_MODEL in ble_text_sensor.TEXT_SENSORS
        assert ble_text_sensor.CONF_ALARMS in ble_text_sensor.TEXT_SENSORS
        assert len(ble_text_sensor.TEXT_SENSORS) == 7


class TestSeplosBmsBleSwitchConstants:
    def test_switches_dict(self):
        assert ble_switch.CONF_DISCHARGING in ble_switch.SWITCHES
        assert ble_switch.CONF_CHARGING in ble_switch.SWITCHES
        assert ble_switch.CONF_CURRENT_LIMIT in ble_switch.SWITCHES
        assert ble_switch.CONF_HEATING in ble_switch.SWITCHES
        assert len(ble_switch.SWITCHES) == 4


class TestSeplosBmsV3BleSensorLists:
    def test_sensor_defs_completeness(self):
        assert "total_voltage" in v3_sensor.SENSOR_DEFS
        assert len(v3_sensor.SENSOR_DEFS) == 36

    def test_binary_sensor_defs_dict(self):
        assert len(v3_binary_sensor.BINARY_SENSOR_DEFS) == 7


class TestSeplosBmsV3BleTextSensorConstants:
    def test_text_sensors_list(self):
        assert v3_text_sensor.CONF_PROBLEM in v3_text_sensor.TEXT_SENSORS
        assert v3_text_sensor.CONF_FACTORY_NAME in v3_text_sensor.TEXT_SENSORS
        assert len(v3_text_sensor.TEXT_SENSORS) == 6
