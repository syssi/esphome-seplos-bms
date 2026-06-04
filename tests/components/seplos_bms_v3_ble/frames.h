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

// EIA idle payload – identical to EIA_DATA except current = 0 A (neither charging nor discharging)
static const std::vector<uint8_t> EIA_DATA_IDLE = {
    0x14, 0xA0, 0x00, 0x00,  // [0-3]   Pack Voltage = 5280 → 52.80 V
    0x00, 0x00, 0x00, 0x00,  // [4-7]   Current = 0 → 0.0 A (idle)
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

// EIC with active alarm in data[2] (cell temperature event → "Problem detected")
static const std::vector<uint8_t> EIC_DATA_WITH_PROBLEM = {
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// EIC with only operating-status codes set (real capture): system state = standby (TB09,
// byte 0) and FET event = charge+discharge FETs on (TB07, byte 7). Must NOT be a problem.
static const std::vector<uint8_t> EIC_DATA_STATUS_ONLY = {
    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00,
};

// EIC with a hard fault (TB15, byte 9, Bit1 = AFE fault) — previously dropped by the mask.
static const std::vector<uint8_t> EIC_DATA_HARD_FAULT = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
};

// EIC with only "cell temperature low heating" status (TB04, byte 3, Bit6) — not a fault.
static const std::vector<uint8_t> EIC_DATA_HEATING = {
    0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// EIC with an ambient/power temperature fault (TB04, byte 3, Bit0) — unlike the
// heating status bit this is a real fault, so it must fold into the temperature
// event code and raise temperature protection.
static const std::vector<uint8_t> EIC_DATA_AMBIENT_TEMP = {
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// SPA captures retained for upcoming sensor tests (no consumer yet).
//
// SPA payload, first request (106 bytes) – System Parameters, registers 0x1300–0x1334
// Modbus function 0x04, captured from an SG16S200A-SP57B-A (16S LFP). Plain big-endian UINT16.
//
// Reg     Field                          Raw     Decoded
// 0x1300  Temperature Sensor Count        0x0004  4
// 0x1301  Cell Count                      0x0010  16
// 0x1305  Pack Overvoltage Protection     0x1680  57.60 V
// 0x1309  Pack Undervoltage Protection    0x10E0  43.20 V
// 0x130D  Cell Overvoltage Protection     0x0E42  3650 mV
// 0x1311  Cell Undervoltage Protection    0x0A8C  2700 mV
// 0x1313  Cell Difference Protection      0x03E8  1000 mV
static const std::vector<uint8_t> SPA_DATA_1 = {
    0x00, 0x04, 0x00, 0x10, 0x15, 0x18, 0x15, 0xE0, 0x15, 0x18, 0x16, 0x80, 0x12, 0xC0, 0x12, 0x20, 0x12, 0xC0,
    0x10, 0xE0, 0x0D, 0x48, 0x0D, 0xAC, 0x0D, 0x48, 0x0E, 0x42, 0x0C, 0x1C, 0x0B, 0x54, 0x0C, 0x1C, 0x0A, 0x8C,
    0x07, 0xD0, 0x03, 0xE8, 0x01, 0xF4, 0x00, 0xCB, 0x00, 0xCD, 0x00, 0xD2, 0x00, 0x64, 0x01, 0x2C, 0x01, 0x2C,
    0xFF, 0x35, 0xFF, 0x33, 0xFF, 0x2E, 0x00, 0x64, 0xFE, 0xA2, 0x01, 0x2C, 0xFE, 0xD4, 0x00, 0x6A, 0x02, 0x58,
    0x00, 0x05, 0x0B, 0xB8, 0x00, 0xCD, 0x00, 0x0A, 0x0D, 0xAC, 0x0D, 0x48, 0x05, 0xDC, 0x03, 0xE8, 0x03, 0x20,
    0x00, 0xC8, 0x00, 0x1E, 0x0C, 0x81, 0x0C, 0x9F, 0x0C, 0x9F, 0x0C, 0xD1, 0x0A, 0xDD, 0x0A, 0xC9,
};

// SPA payload, second request (106 bytes) – System Parameters, registers 0x1335–0x1369
//
// Reg     Field                          Raw     Decoded
// 0x1350  Balancing Start Voltage          0x0D48  3400 mV
// 0x1351  Balancing Start Difference       0x0032  50 mV
// 0x1355  Low State Of Charge Alarm                   0x0032  5.0 %
// 0x1358  Rated Capacity                  0x80E8  330.00 Ah
// 0x1359  Total Capacity                  0x80E8  330.00 Ah
// 0x1366  PCS Charge Current Limit        0x00B4  180 A
// 0x1367  PCS Discharge Current Limit     0xFF4C  180 A
static const std::vector<uint8_t> SPA_DATA_2 = {
    0x0A, 0xBF, 0x0A, 0xAB, 0x0C, 0x9F, 0x0C, 0xD1, 0x0C, 0xD1, 0x0D, 0x03, 0x0A, 0xC9, 0x0A, 0xAB, 0x0A, 0xB5,
    0x0A, 0x8D, 0x0C, 0x81, 0x0C, 0x9F, 0x0C, 0xD1, 0x0D, 0x03, 0x0A, 0xC9, 0x0A, 0xAB, 0x0A, 0xAB, 0x0A, 0x47,
    0x0D, 0xFD, 0x0E, 0x61, 0x0D, 0xFD, 0x0E, 0xF7, 0x0B, 0x0F, 0x0A, 0xAB, 0x0C, 0x9F, 0x0A, 0xAB, 0x00, 0x0A,
    0x0D, 0x48, 0x00, 0x32, 0x00, 0x1E, 0x03, 0xC0, 0x00, 0x50, 0x00, 0x32, 0x00, 0x32, 0x00, 0x1E, 0x80, 0xE8,
    0x80, 0xE8, 0x5C, 0x2B, 0x00, 0x30, 0x00, 0x3C, 0x00, 0xF0, 0x00, 0x0A, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0D,
    0x00, 0x00, 0x01, 0xF4, 0x01, 0x2C, 0x16, 0x80, 0x00, 0xB4, 0xFF, 0x4C, 0x00, 0x00, 0x00, 0x08,
};

// PCT payload (72 bytes) – Protocol Control Type, registers 0x1800–0x1823
// (function 0x04, real capture from SG16S200A-SP57B-A).
//
// Reg            Field                  Offset  Decoded
// 0x1802–0x1811  Inverter Name          4       "SUN-3.6K-SG03P1-EU"
// 0x1812–0x1821  Protocol Support Name  36      "Pylon CAN Protocol"
// 0x1822         Protocol Version       68      "20"
static const std::vector<uint8_t> PCT_DATA = {
    0x00, 0x00,                                      // [0-1]   Protocol Type Switch = 0
    0x01, 0xF4,                                      // [2-3]   Baud Rate = 500
    0x53, 0x55, 0x4E, 0x2D, 0x33, 0x2E, 0x36, 0x4B,  // [4-11]  Inverter Name "SUN-3.6K
    0x2D, 0x53, 0x47, 0x30, 0x33, 0x50, 0x31, 0x2D,  // [12-19]          -SG03P1-
    0x45, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // [20-27]          EU"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // [28-35]
    0x50, 0x79, 0x6C, 0x6F, 0x6E, 0x20, 0x43, 0x41,  // [36-43] Protocol Support Name "Pylon CA
    0x4E, 0x20, 0x50, 0x72, 0x6F, 0x74, 0x6F, 0x63,  // [44-51]                       N Protoc
    0x6F, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // [52-59]                       ol"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // [60-67]
    0x32, 0x30,                                      // [68-69] Protocol Version = "20"
    0x00, 0x00,                                      // [70-71] Protocol Pre Switch = 0
};

}  // namespace esphome::seplos_bms_v3_ble::testing
