#pragma once
#include <cstdint>
#include <vector>

namespace esphome::seplos_bms_v3_ble::testing {

// EIA payload (52 bytes) – EMS Info A, system-level aggregated data
// Modbus function 0x04, registers 0x2000–0x2019 (26 registers × 2 bytes = 52 bytes)
//
// 32-bit values use word-swapped big endian:
//   bytes [i, i+1, i+2, i+3] → value = (d[i+2]<<24)|(d[i+3]<<16)|(d[i]<<8)|d[i+1]
// 16-bit values use standard big endian:
//   bytes [i, i+1] → value = (d[i]<<8)|d[i+1]
//
// Reg     Offset  Field                    Unit    Raw    Decoded
// 0x2000  [0-3]   Pack Voltage             10mV    5280   52.80 V
// 0x2002  [4-7]   Current (INT32)          100mA   100    +10.0 A (charging)
// 0x2004  [8-11]  Remaining Capacity       10mAH   15000  150.0 Ah
// 0x2006  [12-15] Total Capacity           10mAH   20000  200.0 Ah
// 0x2008  [16-19] Total Discharge Capacity 10AH    1000   10000 Ah (cumulative, 50 cycles)
// 0x200A  [20-23] Rated Capacity           10mAH   20000  200.0 Ah
// 0x200C  [24-27] Online Pack Flag         —       1      pack 1 online
// 0x200E  [28-31] Protected Pack bit       —       0      none
// 0x2010  [32-35] Max Discharge Current    100mA   1000   100.0 A
// 0x2012  [36-39] Max Charge Current       100mA   500    50.0 A
// 0x2014  [40-41] Suggest Pack OV          100mV   576    57.6 V
// 0x2015  [42-43] Suggest Pack UV          100mV   464    46.4 V
// 0x2016  [44-45] System Pack No.          —       1      1
// 0x2017  [46-47] Cycle                    —       50     50
// 0x2018  [48-49] SOC                      0.1%    750    75.0 %
// 0x2019  [50-51] SOH                      0.1%    980    98.0 %
static const std::vector<uint8_t> EIA_DATA = {
    0x14, 0xA0, 0x00, 0x00,  // [0-3]   Pack Voltage = 5280 → 52.80 V
    0x00, 0x64, 0x00, 0x00,  // [4-7]   Current = 100 → +10.0 A (charging)
    0x3A, 0x98, 0x00, 0x00,  // [8-11]  Remaining Capacity = 15000 → 150.0 Ah
    0x4E, 0x20, 0x00, 0x00,  // [12-15] Total Capacity = 20000 → 200.0 Ah
    0x03, 0xE8, 0x00, 0x00,  // [16-19] Total Discharge Capacity = 1000 → 10000 Ah
    0x4E, 0x20, 0x00, 0x00,  // [20-23] Rated Capacity = 20000 → 200.0 Ah
    0x00, 0x01, 0x00, 0x00,  // [24-27] Online Pack Flag = 1
    0x00, 0x00, 0x00, 0x00,  // [28-31] Protected Pack bit = 0
    0x03, 0xE8, 0x00, 0x00,  // [32-35] Max Discharge Current = 1000 → 100.0 A
    0x01, 0xF4, 0x00, 0x00,  // [36-39] Max Charge Current = 500 → 50.0 A
    0x02, 0x40,              // [40-41] Suggest Pack OV = 576 → 57.6 V
    0x01, 0xD0,              // [42-43] Suggest Pack UV = 464 → 46.4 V
    0x00, 0x01,              // [44-45] System Pack No. = 1
    0x00, 0x32,              // [46-47] Cycle = 50
    0x02, 0xEE,              // [48-49] SOC = 750 → 75.0 %
    0x03, 0xD4,              // [50-51] SOH = 980 → 98.0 %
};

// EIB payload (52 bytes) – EMS Info B, extremal cell/pack statistics
// Modbus function 0x04, registers 0x2100–0x2119 (26 registers × 2 bytes = 52 bytes)
//
// Note: temperature registers (0x2108–0x210A, 0x210D–0x210F) use 0.1K encoding
//       despite the XZH spec listing "1℃" — verified against real device captures.
//       Conversion: °C = (raw − 2731.5) / 10
//
// Reg     Offset  Field                    Unit    Raw    Decoded
// 0x2100  [0-1]   Max Cell Voltage         1mV     3340   3.340 V
// 0x2101  [2-3]   Min Cell Voltage         1mV     3280   3.280 V
// 0x2102  [4-5]   Max Cell Voltage Id      —       4      cell 4
// 0x2103  [6-7]   Min Cell Voltage Id      —       11     cell 11
// 0x2104  [8-9]   Max Pack Voltage         10mV    5300   53.00 V
// 0x2105  [10-11] Min Pack Voltage         10mV    5280   52.80 V
// 0x2106  [12-13] Max Pack Voltage Id      —       2      pack 2
// 0x2107  [14-15] Min Pack Voltage Id      —       1      pack 1
// 0x2108  [16-17] Max Cell Temperature     0.1K    2992   26.05 °C
// 0x2109  [18-19] Min Cell Temperature     0.1K    2972   24.05 °C
// 0x210A  [20-21] Avg Cell Temperature     0.1K    2982   25.05 °C
// 0x210B  [22-23] Max Cell Temperature Id  —       3      cell 3
// 0x210C  [24-25] Min Cell Temperature Id  —       7      cell 7
//         [26-51] (remaining registers not decoded)
static const std::vector<uint8_t> EIB_DATA = {
    0x0D, 0x0C,                                      // [0-1]   Max Cell Voltage = 3340 → 3.340 V
    0x0C, 0xD0,                                      // [2-3]   Min Cell Voltage = 3280 → 3.280 V
    0x00, 0x04,                                      // [4-5]   Max Cell Voltage Id = 4
    0x00, 0x0B,                                      // [6-7]   Min Cell Voltage Id = 11
    0x14, 0xB4,                                      // [8-9]   Max Pack Voltage = 5300 → 53.00 V
    0x14, 0xA0,                                      // [10-11] Min Pack Voltage = 5280 → 52.80 V
    0x00, 0x02,                                      // [12-13] Max Pack Voltage Id = 2
    0x00, 0x01,                                      // [14-15] Min Pack Voltage Id = 1
    0x0B, 0xB0,                                      // [16-17] Max Cell Temp = 2992 → 26.05 °C
    0x0B, 0x9C,                                      // [18-19] Min Cell Temp = 2972 → 24.05 °C
    0x0B, 0xA6,                                      // [20-21] Avg Cell Temp = 2982 → 25.05 °C
    0x00, 0x03,                                      // [22-23] Max Temp Cell Id = 3
    0x00, 0x07,                                      // [24-25] Min Temp Cell Id = 7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // [26-33] (not decoded)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // [34-41] (not decoded)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // [42-49] (not decoded)
    0x00, 0x00,                                      // [50-51] (not decoded)
};

// EIA discharging payload – identical to EIA_DATA except current = -10.0 A
// Current (Reg 0x2002, INT32, 100mA): raw = -100 = 0xFFFFFF9C
//   word-swapped bytes: [FF 9C FF FF]
// Derived: power = 52.80 * (-10.0) = -528.0 W
//          discharging_power = 528.0 W, charging_power = 0 W
//          runtime = 150.0 Ah / 10.0 A = 15.0 h
static const std::vector<uint8_t> EIA_DATA_DISCHARGING = {
    0x14, 0xA0, 0x00, 0x00,  // [0-3]   Pack Voltage = 5280 → 52.80 V
    0xFF, 0x9C, 0xFF, 0xFF,  // [4-7]   Current = -100 → -10.0 A (discharging)
    0x3A, 0x98, 0x00, 0x00,  // [8-11]  Remaining Capacity = 15000 → 150.0 Ah
    0x4E, 0x20, 0x00, 0x00,  // [12-15] Total Capacity = 20000 → 200.0 Ah
    0x03, 0xE8, 0x00, 0x00,  // [16-19] Total Discharge Capacity = 1000 → 10000 Ah
    0x4E, 0x20, 0x00, 0x00,  // [20-23] Rated Capacity = 20000 → 200.0 Ah
    0x00, 0x01, 0x00, 0x00,  // [24-27] Online Pack Flag = 1
    0x00, 0x00, 0x00, 0x00,  // [28-31] Protected Pack bit = 0
    0x03, 0xE8, 0x00, 0x00,  // [32-35] Max Discharge Current = 1000 → 100.0 A
    0x01, 0xF4, 0x00, 0x00,  // [36-39] Max Charge Current = 500 → 50.0 A
    0x02, 0x40,              // [40-41] Suggest Pack OV = 576 → 57.6 V
    0x01, 0xD0,              // [42-43] Suggest Pack UV = 464 → 46.4 V
    0x00, 0x01,              // [44-45] System Pack No. = 1
    0x00, 0x32,              // [46-47] Cycle = 50
    0x02, 0xEE,              // [48-49] SOC = 750 → 75.0 %
    0x03, 0xD4,              // [50-51] SOH = 980 → 98.0 %
};

// EIC payload (10 bytes) – EMS Info C, system event/alarm codes
// Modbus function 0x01 (read coils), register 0x2200
static const std::vector<uint8_t> EIC_DATA_NO_PROBLEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// EIC with active alarm in data[2] (masked value != 0 → "Problem detected")
static const std::vector<uint8_t> EIC_DATA_WITH_PROBLEM = {
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

}  // namespace esphome::seplos_bms_v3_ble::testing
