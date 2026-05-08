#pragma once
#include <cstdint>
#include <vector>

namespace esphome::seplos_bms_v3_ble::testing {

// EIA payload (52 bytes) – system-level energy information
// decode_eia_data_ reads bytes 0..51 with seplos_get_32bit (word-swapped BE) and seplos_get_16bit
// Word-swapped BE 32-bit: value = (data[i+2]<<24)|(data[i+3]<<16)|(data[i+0]<<8)|data[i+1]
// Decoded values:
//   total_voltage    = 5280 * 0.01   = 52.80 V    (bytes 0-3)
//   current          = 100 * 0.1     = +10.0 A    (bytes 4-7,  charging)
//   power            = 52.80 * 10.0  = 528.0 W
//   charging_power   = 528.0 W,  discharging_power = 0 W
//   cycle_charge     = 100000 * 0.01 = 1000.0 Ah  (bytes 8-11)
//   capacity_remaining = 15000 * 0.01 = 150.0 Ah  (bytes 16-19)
//   total_capacity   = 20000 * 0.01  = 200.0 Ah   (bytes 20-23)
//   max_discharge_current = 1000 * 0.1 = 100.0 A  (bytes 32-35)
//   max_charge_current    = 500 * 0.1  = 50.0 A   (bytes 36-39)
//   rated_capacity   = 20000 * 0.01  = 200.0 Ah   (bytes 40-43)
//   pack_count       = 1                           (bytes 44-45)
//   charging_cycles  = 50                          (bytes 46-47)
//   state_of_charge  = 750 * 0.1     = 75.0 %     (bytes 48-49)
//   state_of_health  = 980 * 0.1     = 98.0 %     (bytes 50-51)
static const std::vector<uint8_t> EIA_DATA = {
    0x14, 0xA0, 0x00, 0x00,  // [0-3]   total_voltage = 5280 → 52.80 V
    0x00, 0x64, 0x00, 0x00,  // [4-7]   current = 100 → +10.0 A (charging)
    0x86, 0xA0, 0x00, 0x01,  // [8-11]  cycle_charge = 100000 → 1000.0 Ah
    0x00, 0x00, 0x00, 0x00,  // [12-15] unused
    0x3A, 0x98, 0x00, 0x00,  // [16-19] capacity_remaining = 15000 → 150.0 Ah
    0x4E, 0x20, 0x00, 0x00,  // [20-23] total_capacity = 20000 → 200.0 Ah
    0x00, 0x00, 0x00, 0x00,  // [24-27] unused
    0x00, 0x00, 0x00, 0x00,  // [28-31] unused
    0x03, 0xE8, 0x00, 0x00,  // [32-35] max_discharge_current = 1000 → 100.0 A
    0x01, 0xF4, 0x00, 0x00,  // [36-39] max_charge_current = 500 → 50.0 A
    0x4E, 0x20, 0x00, 0x00,  // [40-43] rated_capacity = 20000 → 200.0 Ah
    0x00, 0x01,              // [44-45] pack_count = 1
    0x00, 0x32,              // [46-47] charging_cycles = 50
    0x02, 0xEE,              // [48-49] state_of_charge = 750 → 75.0 %
    0x03, 0xD4,              // [50-51] state_of_health = 980 → 98.0 %
};

// EIB payload (52 bytes) – extremal cell/pack statistics
// All values use seplos_get_16bit (big-endian 16-bit)
// Decoded values:
//   max_cell_voltage    = 3340 * 0.001 = 3.340 V   (bytes 0-1)
//   min_cell_voltage    = 3280 * 0.001 = 3.280 V   (bytes 2-3)
//   max_voltage_cell_id = 4                         (bytes 4-5)
//   min_voltage_cell_id = 11                        (bytes 6-7)
//   max_pack_voltage    = 5300 * 0.01  = 53.00 V   (bytes 8-9)
//   min_pack_voltage    = 5280 * 0.01  = 52.80 V   (bytes 10-11)
//   max_pack_voltage_id = 2                         (bytes 12-13)
//   min_pack_voltage_id = 1                         (bytes 14-15)
//   max_cell_temperature = (2992 - 2731.5) / 10 = 26.05 °C  (bytes 16-17)
//   min_cell_temperature = (2972 - 2731.5) / 10 = 24.05 °C  (bytes 18-19)
//   avg_cell_temperature = (2982 - 2731.5) / 10 = 25.05 °C  (bytes 20-21)
//   max_temperature_cell_id = 3                     (bytes 22-23)
//   min_temperature_cell_id = 7                     (bytes 24-25)
//   delta_voltage = 3.340 - 3.280 = 0.060 V
static const std::vector<uint8_t> EIB_DATA = {
    0x0D, 0x0C,                                      // [0-1]   max_cell_voltage = 3340 → 3.340 V
    0x0C, 0xD0,                                      // [2-3]   min_cell_voltage = 3280 → 3.280 V
    0x00, 0x04,                                      // [4-5]   max_voltage_cell_id = 4
    0x00, 0x0B,                                      // [6-7]   min_voltage_cell_id = 11
    0x14, 0xB4,                                      // [8-9]   max_pack_voltage = 5300 → 53.00 V
    0x14, 0xA0,                                      // [10-11] min_pack_voltage = 5280 → 52.80 V
    0x00, 0x02,                                      // [12-13] max_pack_voltage_id = 2
    0x00, 0x01,                                      // [14-15] min_pack_voltage_id = 1
    0x0B, 0xB0,                                      // [16-17] max_cell_temperature = 2992 → 26.05 °C
    0x0B, 0x9C,                                      // [18-19] min_cell_temperature = 2972 → 24.05 °C
    0x0B, 0xA6,                                      // [20-21] avg_cell_temperature = 2982 → 25.05 °C
    0x00, 0x03,                                      // [22-23] max_temperature_cell_id = 3
    0x00, 0x07,                                      // [24-25] min_temperature_cell_id = 7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // [26-33] padding
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // [34-41] padding
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // [42-49] padding
    0x00, 0x00,                                      // [50-51] padding
};

// EIC payload (10 bytes) – no active problems
// problem_code = 0 → "No problems"
static const std::vector<uint8_t> EIC_DATA_NO_PROBLEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// EIC payload (10 bytes) – problem present in high bytes (data[2] != 0)
// problem_code after masking != 0 → "Problem detected"
static const std::vector<uint8_t> EIC_DATA_WITH_PROBLEM = {
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

}  // namespace esphome::seplos_bms_v3_ble::testing
