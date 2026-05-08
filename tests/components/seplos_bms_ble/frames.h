#pragma once
#include <cstdint>
#include <vector>

namespace esphome::seplos_bms_ble::testing {

// Single machine data frame (function 0x61), 8 cells, 4 temperatures
// Decoded key values:
//   cell[1]=3.304V  cell[2]=3.306V  cell[3]=3.302V (min)  cell[4]=3.308V (max)
//   cell[5]=3.304V  cell[6]=3.305V  cell[7]=3.303V  cell[8]=3.307V
//   min=3.302V (cell 3)  max=3.308V (cell 4)  avg=3.305V  delta=0.006V
//   cell_temp[1]=25.1°C  cell_temp[2]=24.5°C  avg_cell_temp=24.8°C
//   ambient_temp=30.0°C  mosfet_temp=35.0°C
//   current=-5.00A  total_voltage=48.00V  power=-240.00W
//   capacity_remaining=100.00Ah  battery_capacity=200.00Ah
//   state_of_charge=50.0%  nominal_capacity=200.00Ah
//   charging_cycles=100  state_of_health=99.9%  port_voltage=48.00V
//   charging=false  discharging=true
//   switch_status=0x03: discharge_switch=on  charge_switch=on
//   all alarm events = 0x00  consolidated_alarms="No alarms"
static const std::vector<uint8_t> SINGLE_MACHINE_FRAME = {
    // header (7 bytes)
    0x7E,
    0x00,
    0x00,
    0x61,
    0x00,
    0x00,
    0x50,
    // device addr, reserved, cells = 8
    0x00,
    0x00,
    0x08,
    // 8 cell voltages big-endian (0.001V each)
    0x0C,
    0xE8,  // cell 1: 3304 → 3.304V
    0x0C,
    0xEA,  // cell 2: 3306 → 3.306V
    0x0C,
    0xE6,  // cell 3: 3302 → 3.302V  (min)
    0x0C,
    0xEC,  // cell 4: 3308 → 3.308V  (max)
    0x0C,
    0xE8,  // cell 5: 3304 → 3.304V
    0x0C,
    0xE9,  // cell 6: 3305 → 3.305V
    0x0C,
    0xE7,  // cell 7: 3303 → 3.303V
    0x0C,
    0xEB,  // cell 8: 3307 → 3.307V
    // temperature count = 4 (2 cell + ambient + mosfet)
    0x04,
    // cell_temp1: 2982 → (2982-2731)*0.1 = 25.1°C
    0x0B,
    0xA6,
    // cell_temp2: 2976 → (2976-2731)*0.1 = 24.5°C
    0x0B,
    0xA0,
    // ambient_temp: 3031 → (3031-2731)*0.1 = 30.0°C
    0x0B,
    0xD7,
    // mosfet_temp: 3081 → (3081-2731)*0.1 = 35.0°C
    0x0C,
    0x09,
    // current: -500 (int16) → -5.00A
    0xFE,
    0x0C,
    // total_voltage: 4800 → 48.00V
    0x12,
    0xC0,
    // capacity_remaining: 10000 → 100.00Ah
    0x27,
    0x10,
    // reserved
    0x00,
    // battery_capacity: 20000 → 200.00Ah
    0x4E,
    0x20,
    // state_of_charge: 500 → 50.0%
    0x01,
    0xF4,
    // nominal_capacity: 20000 → 200.00Ah
    0x4E,
    0x20,
    // charging_cycles: 100
    0x00,
    0x64,
    // state_of_health: 999 → 99.9%
    0x03,
    0xE7,
    // port_voltage: 4800 → 48.00V
    0x12,
    0xC0,
    // cell voltage alarms (8 bytes, one per cell)
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    // cell temp alarms (2 bytes for 2 cell temps)
    0x00,
    0x00,
    // ambient alarm, mosfet alarm
    0x00,
    0x00,
    // current/voltage alarm, total voltage alarm
    0x00,
    0x00,
    // system_status = 0x03 (discharge+charge active)
    0x03,
    // switch_status = 0x03 (discharge_switch=1, charge_switch=1)
    0x03,
    // custom_alarm_volume = 8
    0x08,
    // alarm events 1-8 (all 0x00)
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    // balancing state (1 byte for 8 cells)
    0x00,
    // disconnected state (1 byte)
    0x00,
    // padding to 100 bytes
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

}  // namespace esphome::seplos_bms_ble::testing
