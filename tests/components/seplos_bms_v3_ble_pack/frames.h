#pragma once
#include <cstdint>
#include <vector>

namespace esphome::seplos_bms_v3_ble_pack::testing {

// PIA frame for pack address 0x01: voltage, current, battery level, cycle count
// Frame: [device=0x01, function=0x04, data_len=0x22=34, payload(34 bytes), CRC_L, CRC_H]
// Decoded values:
//   pack_voltage  = 5280 * 0.01  = 52.80 V
//   pack_current  = -500 * 0.01  = -5.00 A  (int16)
//   battery_level = 755 * 0.1    = 75.5 %   (byte offset 10)
//   cycle         = 25            = 25       (byte offset 14)
static const std::vector<uint8_t> PACK_PIA_FRAME = {
    0x01, 0x04, 0x22,  // device, function, data_len=34
    // payload (34 bytes)
    0x14, 0xA0,              // [0-1]   pack_voltage = 5280 → 52.80 V
    0xFE, 0x0C,              // [2-3]   pack_current = -500 (int16) → -5.00 A
    0x00, 0x00,              // [4-5]
    0x00, 0x00,              // [6-7]
    0x00, 0x00,              // [8-9]
    0x02, 0xF3,              // [10-11] battery_level = 755 → 75.5 %
    0x00, 0x00,              // [12-13]
    0x00, 0x19,              // [14-15] cycle = 25
    0x00, 0x00, 0x00, 0x00,  // [16-19]
    0x00, 0x00, 0x00, 0x00,  // [20-23]
    0x00, 0x00, 0x00, 0x00,  // [24-27]
    0x00, 0x00, 0x00, 0x00,  // [28-31]
    0x00, 0x00,              // [32-33]
    0x00, 0x00,              // CRC placeholder
};

// PIB frame for pack address 0x01: 16 cell voltages + 4 temperatures
// Frame: [device=0x01, function=0x04, data_len=0x34=52, payload(52 bytes), CRC_L, CRC_H]
// Decoded values:
//   cell[0]  = 3310 * 0.001 = 3.310 V
//   cell[1]  = 3300 * 0.001 = 3.300 V
//   cell[2]  = 3290 * 0.001 = 3.290 V
//   cell[3]  = 3320 * 0.001 = 3.320 V
//   cell[4..15] = 3300 * 0.001 = 3.300 V
//   temp[0]  = (2982 - 2731.5) * 0.1 = 25.05 °C
//   temp[1]  = (3031 - 2731.5) * 0.1 = 29.95 °C
//   temp[2]  = (2932 - 2731.5) * 0.1 = 20.05 °C
//   temp[3]  = (2882 - 2731.5) * 0.1 = 15.05 °C
static const std::vector<uint8_t> PACK_PIB_FRAME = {
    0x01, 0x04, 0x34,  // device, function, data_len=52
    // payload (52 bytes)
    // 16 cell voltages × 2 bytes each = bytes [0-31]
    0x0C, 0xEE,                                      // cell[0]  = 3310 → 3.310 V
    0x0C, 0xE4,                                      // cell[1]  = 3300 → 3.300 V
    0x0C, 0xDA,                                      // cell[2]  = 3290 → 3.290 V
    0x0C, 0xF8,                                      // cell[3]  = 3320 → 3.320 V
    0x0C, 0xE4, 0x0C, 0xE4, 0x0C, 0xE4, 0x0C, 0xE4,  // cell[4-7]  = 3300
    0x0C, 0xE4, 0x0C, 0xE4, 0x0C, 0xE4, 0x0C, 0xE4,  // cell[8-11] = 3300
    0x0C, 0xE4, 0x0C, 0xE4, 0x0C, 0xE4, 0x0C, 0xE4,  // cell[12-15] = 3300
    // 4 temperatures × 2 bytes each = bytes [32-39]
    0x0B, 0xA6,  // temp[0] = 2982 → 25.05 °C
    0x0B, 0xD7,  // temp[1] = 3031 → 29.95 °C
    0x0B, 0x74,  // temp[2] = 2932 → 20.05 °C
    0x0B, 0x42,  // temp[3] = 2882 → 15.05 °C
    // env temp = bytes [40-41], rest padding to 52 bytes
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // CRC placeholder
};

}  // namespace esphome::seplos_bms_v3_ble_pack::testing
