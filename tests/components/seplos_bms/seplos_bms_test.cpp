#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

namespace esphome::seplos_bms::testing {

// ── Cell voltages ─────────────────────────────────────────────────────────────

TEST(SeplosBmsTelemetryTest, CellVoltages) {
  TestableSeplosBms bms;
  sensor::Sensor cells[16];
  for (int i = 0; i < 16; i++)
    bms.set_cell_voltage_sensor(i, &cells[i]);

  bms.on_seplos_modbus_data(TELEMETRY_FRAME);

  EXPECT_NEAR(cells[0].state, 3.287f, 0.001f);
  EXPECT_NEAR(cells[1].state, 3.305f, 0.001f);
  EXPECT_NEAR(cells[2].state, 3.316f, 0.001f);
  EXPECT_NEAR(cells[3].state, 3.286f, 0.001f);
  EXPECT_NEAR(cells[4].state, 3.311f, 0.001f);
  EXPECT_NEAR(cells[15].state, 3.288f, 0.001f);
}

TEST(SeplosBmsTelemetryTest, CellVoltageStats) {
  TestableSeplosBms bms;
  sensor::Sensor min_v, max_v, min_cell, max_cell, delta, avg;
  bms.set_min_cell_voltage_sensor(&min_v);
  bms.set_max_cell_voltage_sensor(&max_v);
  bms.set_min_voltage_cell_sensor(&min_cell);
  bms.set_max_voltage_cell_sensor(&max_cell);
  bms.set_delta_cell_voltage_sensor(&delta);
  bms.set_average_cell_voltage_sensor(&avg);

  bms.on_seplos_modbus_data(TELEMETRY_FRAME);

  EXPECT_NEAR(min_v.state, 3.286f, 0.001f);
  EXPECT_NEAR(max_v.state, 3.316f, 0.001f);
  EXPECT_FLOAT_EQ(min_cell.state, 4.0f);
  EXPECT_FLOAT_EQ(max_cell.state, 3.0f);
  EXPECT_NEAR(delta.state, 0.030f, 0.001f);
  EXPECT_NEAR(avg.state, 3.300f, 0.001f);
}

// ── Temperatures ──────────────────────────────────────────────────────────────

TEST(SeplosBmsTelemetryTest, Temperatures) {
  TestableSeplosBms bms;
  sensor::Sensor t[6];
  for (int i = 0; i < 6; i++)
    bms.set_temperature_sensor(i, &t[i]);

  bms.on_seplos_modbus_data(TELEMETRY_FRAME);

  EXPECT_NEAR(t[0].state, 25.1f, 0.1f);
  EXPECT_NEAR(t[1].state, 24.5f, 0.1f);
  EXPECT_NEAR(t[2].state, 23.6f, 0.1f);
  EXPECT_NEAR(t[3].state, 25.1f, 0.1f);
  EXPECT_NEAR(t[4].state, 25.0f, 0.1f);
  EXPECT_NEAR(t[5].state, 24.7f, 0.1f);
}

// ── Current and power ─────────────────────────────────────────────────────────

TEST(SeplosBmsTelemetryTest, CurrentAndPower) {
  TestableSeplosBms bms;
  sensor::Sensor current, power, charging_power, discharging_power;
  bms.set_current_sensor(&current);
  bms.set_power_sensor(&power);
  bms.set_charging_power_sensor(&charging_power);
  bms.set_discharging_power_sensor(&discharging_power);

  bms.on_seplos_modbus_data(TELEMETRY_FRAME);

  EXPECT_NEAR(current.state, -6.76f, 0.01f);
  EXPECT_NEAR(power.state, -356.93f, 1.0f);
  EXPECT_NEAR(charging_power.state, 0.0f, 0.01f);
  EXPECT_NEAR(discharging_power.state, 356.93f, 1.0f);
}

// ── Voltage ───────────────────────────────────────────────────────────────────

TEST(SeplosBmsTelemetryTest, TotalVoltage) {
  TestableSeplosBms bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  bms.on_seplos_modbus_data(TELEMETRY_FRAME);

  EXPECT_NEAR(total.state, 52.80f, 0.01f);
}

// ── Capacity ──────────────────────────────────────────────────────────────────

TEST(SeplosBmsTelemetryTest, Capacity) {
  TestableSeplosBms bms;
  sensor::Sensor residual, battery, rated;
  bms.set_residual_capacity_sensor(&residual);
  bms.set_battery_capacity_sensor(&battery);
  bms.set_rated_capacity_sensor(&rated);

  bms.on_seplos_modbus_data(TELEMETRY_FRAME);

  EXPECT_NEAR(residual.state, 133.90f, 0.01f);
  EXPECT_NEAR(battery.state, 170.00f, 0.01f);
  EXPECT_NEAR(rated.state, 180.00f, 0.01f);
}

// ── State of charge and health ────────────────────────────────────────────────

TEST(SeplosBmsTelemetryTest, StateOfChargeAndHealth) {
  TestableSeplosBms bms;
  sensor::Sensor soc, soh, cycles;
  bms.set_state_of_charge_sensor(&soc);
  bms.set_state_of_health_sensor(&soh);
  bms.set_charging_cycles_sensor(&cycles);

  bms.on_seplos_modbus_data(TELEMETRY_FRAME);

  EXPECT_NEAR(soc.state, 78.7f, 0.1f);
  EXPECT_NEAR(soh.state, 100.0f, 0.1f);
  EXPECT_FLOAT_EQ(cycles.state, 70.0f);
}

// ── Port voltage ──────────────────────────────────────────────────────────────

TEST(SeplosBmsTelemetryTest, PortVoltage) {
  TestableSeplosBms bms;
  sensor::Sensor port;
  bms.set_port_voltage_sensor(&port);

  bms.on_seplos_modbus_data(TELEMETRY_FRAME);

  EXPECT_NEAR(port.state, 52.79f, 0.01f);
}

// ── Online status ─────────────────────────────────────────────────────────────

TEST(SeplosBmsTelemetryTest, OnlineStatus) {
  TestableSeplosBms bms;
  binary_sensor::BinarySensor online;
  bms.set_online_status_binary_sensor(&online);

  bms.on_seplos_modbus_data(TELEMETRY_FRAME);

  EXPECT_TRUE(online.state);
}

// ── Null sensors do not crash ─────────────────────────────────────────────────

TEST(SeplosBmsSafetyTest, NullSensorsDoNotCrash) {
  TestableSeplosBms bms;

  EXPECT_NO_FATAL_FAILURE(bms.on_seplos_modbus_data(TELEMETRY_FRAME));
}

}  // namespace esphome::seplos_bms::testing
