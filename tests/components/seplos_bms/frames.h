#pragma once
#include <cstdint>
#include <vector>

namespace esphome::seplos_bms::testing {

// Source: code comment in seplos_bms.cpp
// 16 cells, 6 temperature sensors
// Decoded key values:
//   cell[1]=3.287V  cell[2]=3.305V  cell[3]=3.316V  cell[4]=3.286V (min)
//   cell[5]=3.311V  cell[6]=3.301V  cell[7]=3.297V  cell[8]=3.292V
//   cell[9]=3.305V  cell[10]=3.312V cell[11]=3.304V cell[12]=3.311V
//   cell[13]=3.306V cell[14]=3.290V cell[15]=3.294V cell[16]=3.288V
//   min=3.286V (cell 4)  max=3.316V (cell 3)  avg≈3.300V  delta=0.030V
//   temp[1]=25.1°C  temp[2]=24.5°C  temp[3]=23.6°C  temp[4]=25.1°C
//   temp[5]=25.0°C  temp[6]=24.7°C
//   current=-6.76A  total_voltage=52.80V  power=-356.93W
//   residual_capacity=133.90Ah  battery_capacity=170.00Ah
//   state_of_charge=78.7%  rated_capacity=180.00Ah
//   charging_cycles=70  state_of_health=100.0%  port_voltage=52.79V
static const std::vector<uint8_t> TELEMETRY_FRAME = {
    0x20, 0x00, 0x46, 0x00, 0x10, 0x96, 0x00, 0x01,  // header
    0x10,                                            // cells = 16
    0x0C, 0xD7, 0x0C, 0xE9, 0x0C, 0xF4, 0x0C, 0xD6,  // cells 1-4
    0x0C, 0xEF, 0x0C, 0xE5, 0x0C, 0xE1, 0x0C, 0xDC,  // cells 5-8
    0x0C, 0xE9, 0x0C, 0xF0, 0x0C, 0xE8, 0x0C, 0xEF,  // cells 9-12
    0x0C, 0xEA, 0x0C, 0xDA, 0x0C, 0xDE, 0x0C, 0xD8,  // cells 13-16
    0x06,                                            // temperature sensors = 6
    0x0B, 0xA6, 0x0B, 0xA0, 0x0B, 0x97,              // temps 1-3
    0x0B, 0xA6, 0x0B, 0xA5, 0x0B, 0xA2,              // temps 4-6
    0xFD, 0x5C,                                      // current = -676 * 0.01 = -6.76A
    0x14, 0xA0,                                      // total_voltage = 5280 * 0.01 = 52.80V
    0x34, 0x4E,                                      // residual_capacity = 13390 * 0.01 = 133.90Ah
    0x0A,                                            // custom number (skip)
    0x42, 0x68,                                      // battery_capacity = 17000 * 0.01 = 170.00Ah
    0x03, 0x13,                                      // state_of_charge = 787 * 0.1 = 78.7%
    0x46, 0x50,                                      // rated_capacity = 18000 * 0.01 = 180.00Ah
    0x00, 0x46,                                      // charging_cycles = 70
    0x03, 0xE8,                                      // state_of_health = 1000 * 0.1 = 100.0%
    0x14, 0x9F,                                      // port_voltage = 5279 * 0.01 = 52.79V
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // reserved
};

}  // namespace esphome::seplos_bms::testing
