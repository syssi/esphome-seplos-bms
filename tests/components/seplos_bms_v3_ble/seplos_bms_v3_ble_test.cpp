#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

namespace esphome::seplos_bms_v3_ble::testing {

// ── EIA: voltage, current, power (charging) ───────────────────────────────────

TEST(SeplosBmsV3BleEiaTest, VoltageCurrentPowerCharging) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor voltage, current, power, charging_power, discharging_power;
  binary_sensor::BinarySensor charging, discharging;
  bms.set_total_voltage_sensor(&voltage);
  bms.set_current_sensor(&current);
  bms.set_power_sensor(&power);
  bms.set_charging_power_sensor(&charging_power);
  bms.set_discharging_power_sensor(&discharging_power);
  bms.set_charging_binary_sensor(&charging);
  bms.set_discharging_binary_sensor(&discharging);

  bms.decode_eia(EIA_DATA);

  EXPECT_NEAR(voltage.state, 52.80f, 0.01f);
  EXPECT_NEAR(current.state, 10.0f, 0.01f);
  EXPECT_NEAR(power.state, 528.0f, 1.0f);
  EXPECT_NEAR(charging_power.state, 528.0f, 1.0f);
  EXPECT_NEAR(discharging_power.state, 0.0f, 0.01f);
  EXPECT_TRUE(charging.state);
  EXPECT_FALSE(discharging.state);
}

// ── EIA: voltage, current, power (discharging) ────────────────────────────────

TEST(SeplosBmsV3BleEiaTest, VoltageCurrentPowerDischarging) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor voltage, current, power, charging_power, discharging_power;
  binary_sensor::BinarySensor charging, discharging;
  bms.set_total_voltage_sensor(&voltage);
  bms.set_current_sensor(&current);
  bms.set_power_sensor(&power);
  bms.set_charging_power_sensor(&charging_power);
  bms.set_discharging_power_sensor(&discharging_power);
  bms.set_charging_binary_sensor(&charging);
  bms.set_discharging_binary_sensor(&discharging);

  bms.decode_eia(EIA_DATA_DISCHARGING);

  EXPECT_NEAR(voltage.state, 52.80f, 0.01f);
  EXPECT_NEAR(current.state, -10.0f, 0.01f);
  EXPECT_NEAR(power.state, -528.0f, 1.0f);
  EXPECT_NEAR(charging_power.state, 0.0f, 0.01f);
  EXPECT_NEAR(discharging_power.state, 528.0f, 1.0f);
  EXPECT_FALSE(charging.state);
  EXPECT_TRUE(discharging.state);
}

// 0 A is idle: neither charging nor discharging, both power values 0
TEST(SeplosBmsV3BleEiaTest, VoltageCurrentPowerIdle) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor voltage, current, power, charging_power, discharging_power;
  binary_sensor::BinarySensor charging, discharging;
  bms.set_total_voltage_sensor(&voltage);
  bms.set_current_sensor(&current);
  bms.set_power_sensor(&power);
  bms.set_charging_power_sensor(&charging_power);
  bms.set_discharging_power_sensor(&discharging_power);
  bms.set_charging_binary_sensor(&charging);
  bms.set_discharging_binary_sensor(&discharging);

  bms.decode_eia(EIA_DATA_IDLE);

  EXPECT_NEAR(current.state, 0.0f, 0.01f);
  EXPECT_NEAR(power.state, 0.0f, 0.01f);
  EXPECT_NEAR(charging_power.state, 0.0f, 0.01f);
  EXPECT_NEAR(discharging_power.state, 0.0f, 0.01f);
  EXPECT_FALSE(charging.state);
  EXPECT_FALSE(discharging.state);
}

// ── EIA: capacity registers ───────────────────────────────────────────────────

TEST(SeplosBmsV3BleEiaTest, CapacityRegisters) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor cap_rem, total_cap, rated_cap, cycle_charge;
  bms.set_capacity_remaining_sensor(&cap_rem);
  bms.set_total_capacity_sensor(&total_cap);
  bms.set_rated_capacity_sensor(&rated_cap);
  bms.set_cycle_charge_sensor(&cycle_charge);

  bms.decode_eia(EIA_DATA);

  EXPECT_NEAR(cap_rem.state, 150.0f, 0.01f);
  EXPECT_NEAR(total_cap.state, 200.0f, 0.01f);
  EXPECT_NEAR(rated_cap.state, 200.0f, 0.01f);
  EXPECT_NEAR(cycle_charge.state, 10000.0f, 1.0f);
}

// ── EIA: state registers ──────────────────────────────────────────────────────

TEST(SeplosBmsV3BleEiaTest, StateRegisters) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor soc, soh, cycles, pack_count;
  bms.set_state_of_charge_sensor(&soc);
  bms.set_state_of_health_sensor(&soh);
  bms.set_charging_cycles_sensor(&cycles);
  bms.set_pack_count_sensor(&pack_count);

  bms.decode_eia(EIA_DATA);

  EXPECT_NEAR(soc.state, 75.0f, 0.1f);
  EXPECT_NEAR(soh.state, 98.0f, 0.1f);
  EXPECT_FLOAT_EQ(cycles.state, 50.0f);
  EXPECT_FLOAT_EQ(pack_count.state, 1.0f);
}

// ── EIA: current limit registers ─────────────────────────────────────────────

TEST(SeplosBmsV3BleEiaTest, CurrentLimitRegisters) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor max_disch, max_chg;
  bms.set_max_discharge_current_sensor(&max_disch);
  bms.set_max_charge_current_sensor(&max_chg);

  bms.decode_eia(EIA_DATA);

  EXPECT_NEAR(max_disch.state, 100.0f, 0.1f);
  EXPECT_NEAR(max_chg.state, 50.0f, 0.1f);
}

// ── EIA: derived values ───────────────────────────────────────────────────────

TEST(SeplosBmsV3BleEiaTest, DerivedCycleCapacity) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor cycle_charge, cycles, cycle_cap;
  bms.set_cycle_charge_sensor(&cycle_charge);
  bms.set_charging_cycles_sensor(&cycles);
  bms.set_cycle_capacity_sensor(&cycle_cap);

  bms.decode_eia(EIA_DATA);

  // cycle_capacity = cycle_charge / cycles = 10000 / 50 = 200 Ah/cycle
  EXPECT_NEAR(cycle_cap.state, 200.0f, 0.1f);
}

TEST(SeplosBmsV3BleEiaTest, DerivedRuntime) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor cap_rem, current, runtime;
  bms.set_capacity_remaining_sensor(&cap_rem);
  bms.set_current_sensor(&current);
  bms.set_runtime_sensor(&runtime);

  bms.decode_eia(EIA_DATA_DISCHARGING);

  // runtime = remaining / |current| = 150.0 Ah / 10.0 A = 15.0 h
  EXPECT_NEAR(runtime.state, 15.0f, 0.1f);
}

// ── EIB: cell voltage extremes ────────────────────────────────────────────────

TEST(SeplosBmsV3BleEibTest, CellVoltageExtremes) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor max_v, min_v, max_cell, min_cell, delta;
  bms.set_max_cell_voltage_sensor(&max_v);
  bms.set_min_cell_voltage_sensor(&min_v);
  bms.set_max_voltage_cell_sensor(&max_cell);
  bms.set_min_voltage_cell_sensor(&min_cell);
  bms.set_delta_voltage_sensor(&delta);

  bms.decode_eib(EIB_DATA);

  EXPECT_NEAR(max_v.state, 3.340f, 0.001f);
  EXPECT_NEAR(min_v.state, 3.280f, 0.001f);
  EXPECT_FLOAT_EQ(max_cell.state, 4.0f);
  EXPECT_FLOAT_EQ(min_cell.state, 11.0f);
  EXPECT_NEAR(delta.state, 0.060f, 0.001f);
}

// ── EIB: pack voltage extremes ────────────────────────────────────────────────

TEST(SeplosBmsV3BleEibTest, PackVoltageExtremes) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor max_pack, min_pack, max_pack_id, min_pack_id;
  bms.set_max_pack_voltage_sensor(&max_pack);
  bms.set_min_pack_voltage_sensor(&min_pack);
  bms.set_max_pack_voltage_id_sensor(&max_pack_id);
  bms.set_min_pack_voltage_id_sensor(&min_pack_id);

  bms.decode_eib(EIB_DATA);

  EXPECT_NEAR(max_pack.state, 53.00f, 0.01f);
  EXPECT_NEAR(min_pack.state, 52.80f, 0.01f);
  EXPECT_FLOAT_EQ(max_pack_id.state, 2.0f);
  EXPECT_FLOAT_EQ(min_pack_id.state, 1.0f);
}

// ── EIB: cell temperature extremes ───────────────────────────────────────────

TEST(SeplosBmsV3BleEibTest, CellTemperatureExtremes) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor max_temp, min_temp, avg_temp, max_temp_cell, min_temp_cell;
  bms.set_max_cell_temperature_sensor(&max_temp);
  bms.set_min_cell_temperature_sensor(&min_temp);
  bms.set_average_cell_temperature_sensor(&avg_temp);
  bms.set_max_temperature_cell_sensor(&max_temp_cell);
  bms.set_min_temperature_cell_sensor(&min_temp_cell);

  bms.decode_eib(EIB_DATA);

  // 0.1K encoding: °C = (raw − 2731.5) / 10
  // 2992 → 26.05 °C, 2972 → 24.05 °C, 2982 → 25.05 °C
  EXPECT_NEAR(max_temp.state, 26.05f, 0.05f);
  EXPECT_NEAR(min_temp.state, 24.05f, 0.05f);
  EXPECT_NEAR(avg_temp.state, 25.05f, 0.05f);
  EXPECT_FLOAT_EQ(max_temp_cell.state, 3.0f);
  EXPECT_FLOAT_EQ(min_temp_cell.state, 7.0f);
}

// ── EIC: problem code and text ────────────────────────────────────────────────

TEST(SeplosBmsV3BleEicTest, NoProblems) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor problem_code;
  text_sensor::TextSensor problem_text;
  bms.set_problem_code_sensor(&problem_code);
  bms.set_problem_text_sensor(&problem_text);

  bms.decode_eic(EIC_DATA_NO_PROBLEM);

  EXPECT_FLOAT_EQ(problem_code.state, 0.0f);
  EXPECT_EQ(problem_text.state, "No problems");
}

TEST(SeplosBmsV3BleEicTest, WithProblem) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor problem_code;
  text_sensor::TextSensor problem_text;
  bms.set_problem_code_sensor(&problem_code);
  bms.set_problem_text_sensor(&problem_text);

  bms.decode_eic(EIC_DATA_WITH_PROBLEM);

  EXPECT_NE(problem_code.state, 0.0f);
  EXPECT_EQ(problem_text.state, "Problem detected");
}

// Operating-status codes (system state, FETs on, equalization) must not be a problem.
TEST(SeplosBmsV3BleEicTest, StatusCodesAreNotProblems) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor problem_code;
  text_sensor::TextSensor problem_text;
  bms.set_problem_code_sensor(&problem_code);
  bms.set_problem_text_sensor(&problem_text);

  bms.decode_eic(EIC_DATA_STATUS_ONLY);

  EXPECT_FLOAT_EQ(problem_code.state, 0.0f);
  EXPECT_EQ(problem_text.state, "No problems");
}

// Hard fault codes (byte 9) were dropped by the old mask; they must be detected now.
TEST(SeplosBmsV3BleEicTest, HardFaultDetected) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor problem_code;
  text_sensor::TextSensor problem_text;
  bms.set_problem_code_sensor(&problem_code);
  bms.set_problem_text_sensor(&problem_text);

  bms.decode_eic(EIC_DATA_HARD_FAULT);

  EXPECT_NE(problem_code.state, 0.0f);
  EXPECT_EQ(problem_text.state, "Problem detected");
}

// "Cell temperature low heating" (TB04 bit6) is an operating state, not a fault.
TEST(SeplosBmsV3BleEicTest, HeatingIsNotAProblem) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor problem_code;
  text_sensor::TextSensor problem_text;
  bms.set_problem_code_sensor(&problem_code);
  bms.set_problem_text_sensor(&problem_text);

  bms.decode_eic(EIC_DATA_HEATING);

  EXPECT_FLOAT_EQ(problem_code.state, 0.0f);
  EXPECT_EQ(problem_text.state, "No problems");
}

// ── PCT: inverter / protocol sensors ──────────────────────────────────────────

TEST(SeplosBmsV3BlePctTest, Sensors) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor protocol, baud_rate, protocol_pre_switch;
  text_sensor::TextSensor inverter_name, inverter_protocol_name, inverter_protocol_version;
  bms.set_inverter_protocol_sensor(&protocol);
  bms.set_inverter_baud_rate_sensor(&baud_rate);
  bms.set_inverter_protocol_pre_switch_sensor(&protocol_pre_switch);
  bms.set_inverter_name_text_sensor(&inverter_name);
  bms.set_inverter_protocol_name_text_sensor(&inverter_protocol_name);
  bms.set_inverter_protocol_version_text_sensor(&inverter_protocol_version);

  bms.decode_pct(PCT_DATA);

  EXPECT_FLOAT_EQ(protocol.state, 0.0f);
  EXPECT_FLOAT_EQ(baud_rate.state, 500.0f);
  EXPECT_FLOAT_EQ(protocol_pre_switch.state, 0.0f);
  EXPECT_EQ(inverter_name.state, "SUN-3.6K-SG03P1-EU");
  EXPECT_EQ(inverter_protocol_name.state, "Pylon CAN Protocol");
  EXPECT_EQ(inverter_protocol_version.state, "20");
}

TEST(SeplosBmsV3BlePctTest, NullSensorsDoNotCrash) {
  TestableSeplosBmsV3Ble bms;

  EXPECT_NO_FATAL_FAILURE(bms.decode_pct(PCT_DATA));
}

// ── SPA1: protection threshold sensors (registers 0x1300–0x1334) ───────────────

TEST(SeplosBmsV3BleSpa1Test, CountRegisters) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor temperature_sensor_count, cell_count;
  bms.set_temperature_sensor_count_sensor(&temperature_sensor_count);
  bms.set_cell_count_sensor(&cell_count);

  bms.decode_spa1(SPA_DATA_1);

  EXPECT_FLOAT_EQ(temperature_sensor_count.state, 4.0f);
  EXPECT_FLOAT_EQ(cell_count.state, 16.0f);
}

TEST(SeplosBmsV3BleSpa1Test, PackVoltageThresholds) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor ov_recover, ov_protect, uv_recover, uv_protect;
  bms.set_pack_overvoltage_recovery_voltage_sensor(&ov_recover);
  bms.set_pack_overvoltage_protection_voltage_sensor(&ov_protect);
  bms.set_pack_undervoltage_recovery_voltage_sensor(&uv_recover);
  bms.set_pack_undervoltage_protection_voltage_sensor(&uv_protect);

  bms.decode_spa1(SPA_DATA_1);

  EXPECT_NEAR(ov_recover.state, 54.00f, 0.01f);
  EXPECT_NEAR(ov_protect.state, 57.60f, 0.01f);
  EXPECT_NEAR(uv_recover.state, 48.00f, 0.01f);
  EXPECT_NEAR(uv_protect.state, 43.20f, 0.01f);
}

TEST(SeplosBmsV3BleSpa1Test, CellVoltageThresholds) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor ov_recover, ov_protect, uv_recover, uv_protect, diff;
  bms.set_cell_overvoltage_recovery_voltage_sensor(&ov_recover);
  bms.set_cell_overvoltage_protection_voltage_sensor(&ov_protect);
  bms.set_cell_undervoltage_recovery_voltage_sensor(&uv_recover);
  bms.set_cell_undervoltage_protection_voltage_sensor(&uv_protect);
  bms.set_cell_voltage_difference_protection_sensor(&diff);

  bms.decode_spa1(SPA_DATA_1);

  EXPECT_NEAR(ov_recover.state, 3.400f, 0.001f);
  EXPECT_NEAR(ov_protect.state, 3.650f, 0.001f);
  EXPECT_NEAR(uv_recover.state, 3.100f, 0.001f);
  EXPECT_NEAR(uv_protect.state, 2.700f, 0.001f);
  EXPECT_NEAR(diff.state, 1.000f, 0.001f);
}

TEST(SeplosBmsV3BleSpa1Test, CurrentProtection) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor charge_oc, discharge_oc;
  bms.set_charge_overcurrent_protection_sensor(&charge_oc);
  bms.set_discharge_overcurrent_protection_sensor(&discharge_oc);

  bms.decode_spa1(SPA_DATA_1);

  EXPECT_FLOAT_EQ(charge_oc.state, 210.0f);
  EXPECT_FLOAT_EQ(discharge_oc.state, 210.0f);
}

TEST(SeplosBmsV3BleSpa1Test, TemperatureThresholds) {
  TestableSeplosBmsV3Ble bms;
  sensor::Sensor charge_otp, charge_lta;
  bms.set_charge_overtemperature_protection_sensor(&charge_otp);
  bms.set_charge_low_temperature_alarm_sensor(&charge_lta);

  bms.decode_spa1(SPA_DATA_1);

  EXPECT_NEAR(charge_otp.state, 55.0f, 0.1f);
  EXPECT_NEAR(charge_lta.state, 3.0f, 0.1f);
}

TEST(SeplosBmsV3BleSpa1Test, NullSensorsDoNotCrash) {
  TestableSeplosBmsV3Ble bms;

  EXPECT_NO_FATAL_FAILURE(bms.decode_spa1(SPA_DATA_1));
}

// ── Null sensors do not crash ─────────────────────────────────────────────────

TEST(SeplosBmsV3BleSafetyTest, NullSensorsDoNotCrash) {
  TestableSeplosBmsV3Ble bms;

  EXPECT_NO_FATAL_FAILURE(bms.decode_eia(EIA_DATA));
  EXPECT_NO_FATAL_FAILURE(bms.decode_eia(EIA_DATA_DISCHARGING));
  EXPECT_NO_FATAL_FAILURE(bms.decode_eib(EIB_DATA));
  EXPECT_NO_FATAL_FAILURE(bms.decode_eic(EIC_DATA_NO_PROBLEM));
  EXPECT_NO_FATAL_FAILURE(bms.decode_eic(EIC_DATA_WITH_PROBLEM));
}

}  // namespace esphome::seplos_bms_v3_ble::testing
