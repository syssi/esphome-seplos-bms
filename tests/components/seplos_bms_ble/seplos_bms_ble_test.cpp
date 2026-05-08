#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

namespace esphome::seplos_bms_ble::testing {

// ── Cell voltages ─────────────────────────────────────────────────────────────

TEST(SeplosBmsBleStatusTest, CellVoltages) {
  TestableSeplosBmsBle bms;
  sensor::Sensor cells[8];
  for (int i = 0; i < 8; i++)
    bms.set_cell_voltage_sensor(i, &cells[i]);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_NEAR(cells[0].state, 3.304f, 0.001f);
  EXPECT_NEAR(cells[1].state, 3.306f, 0.001f);
  EXPECT_NEAR(cells[2].state, 3.302f, 0.001f);
  EXPECT_NEAR(cells[3].state, 3.308f, 0.001f);
  EXPECT_NEAR(cells[4].state, 3.304f, 0.001f);
  EXPECT_NEAR(cells[5].state, 3.305f, 0.001f);
  EXPECT_NEAR(cells[6].state, 3.303f, 0.001f);
  EXPECT_NEAR(cells[7].state, 3.307f, 0.001f);
}

TEST(SeplosBmsBleStatusTest, CellVoltageStats) {
  TestableSeplosBmsBle bms;
  sensor::Sensor min_v, max_v, min_cell, max_cell, delta, avg;
  bms.set_min_cell_voltage_sensor(&min_v);
  bms.set_max_cell_voltage_sensor(&max_v);
  bms.set_min_voltage_cell_sensor(&min_cell);
  bms.set_max_voltage_cell_sensor(&max_cell);
  bms.set_delta_cell_voltage_sensor(&delta);
  bms.set_average_cell_voltage_sensor(&avg);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_NEAR(min_v.state, 3.302f, 0.001f);
  EXPECT_NEAR(max_v.state, 3.308f, 0.001f);
  EXPECT_FLOAT_EQ(min_cell.state, 3.0f);
  EXPECT_FLOAT_EQ(max_cell.state, 4.0f);
  EXPECT_NEAR(delta.state, 0.006f, 0.001f);
  EXPECT_NEAR(avg.state, 3.305f, 0.001f);
}

// ── Temperatures ──────────────────────────────────────────────────────────────

TEST(SeplosBmsBleStatusTest, Temperatures) {
  TestableSeplosBmsBle bms;
  sensor::Sensor t0, t1, ambient, mosfet, avg_cell_temp;
  bms.set_temperature_sensor(0, &t0);
  bms.set_temperature_sensor(1, &t1);
  bms.set_ambient_temperature_sensor(&ambient);
  bms.set_mosfet_temperature_sensor(&mosfet);
  bms.set_average_cell_temperature_sensor(&avg_cell_temp);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_NEAR(t0.state, 25.1f, 0.1f);
  EXPECT_NEAR(t1.state, 24.5f, 0.1f);
  EXPECT_NEAR(ambient.state, 30.0f, 0.1f);
  EXPECT_NEAR(mosfet.state, 35.0f, 0.1f);
  EXPECT_NEAR(avg_cell_temp.state, 24.8f, 0.1f);
}

// ── Current and power ─────────────────────────────────────────────────────────

TEST(SeplosBmsBleStatusTest, CurrentAndPower) {
  TestableSeplosBmsBle bms;
  sensor::Sensor current, power, charging_power, discharging_power;
  bms.set_current_sensor(&current);
  bms.set_power_sensor(&power);
  bms.set_charging_power_sensor(&charging_power);
  bms.set_discharging_power_sensor(&discharging_power);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_NEAR(current.state, -5.00f, 0.01f);
  EXPECT_NEAR(power.state, -240.00f, 1.0f);
  EXPECT_NEAR(charging_power.state, 0.0f, 0.01f);
  EXPECT_NEAR(discharging_power.state, 240.00f, 1.0f);
}

// ── Charging/discharging binary sensors ───────────────────────────────────────

TEST(SeplosBmsBleStatusTest, ChargingDischargingBinary) {
  TestableSeplosBmsBle bms;
  binary_sensor::BinarySensor charging, discharging;
  bms.set_charging_binary_sensor(&charging);
  bms.set_discharging_binary_sensor(&discharging);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_FALSE(charging.state);
  EXPECT_TRUE(discharging.state);
}

// ── Total voltage ─────────────────────────────────────────────────────────────

TEST(SeplosBmsBleStatusTest, TotalVoltage) {
  TestableSeplosBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_NEAR(total.state, 48.00f, 0.01f);
}

// ── Capacity ──────────────────────────────────────────────────────────────────

TEST(SeplosBmsBleStatusTest, Capacity) {
  TestableSeplosBmsBle bms;
  sensor::Sensor cap_rem, batt_cap, nom_cap;
  bms.set_capacity_remaining_sensor(&cap_rem);
  bms.set_battery_capacity_sensor(&batt_cap);
  bms.set_nominal_capacity_sensor(&nom_cap);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_NEAR(cap_rem.state, 100.00f, 0.01f);
  EXPECT_NEAR(batt_cap.state, 200.00f, 0.01f);
  EXPECT_NEAR(nom_cap.state, 200.00f, 0.01f);
}

// ── State of charge and health ────────────────────────────────────────────────

TEST(SeplosBmsBleStatusTest, StateOfChargeAndHealth) {
  TestableSeplosBmsBle bms;
  sensor::Sensor soc, soh, cycles;
  bms.set_state_of_charge_sensor(&soc);
  bms.set_state_of_health_sensor(&soh);
  bms.set_charging_cycles_sensor(&cycles);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_NEAR(soc.state, 50.0f, 0.1f);
  EXPECT_NEAR(soh.state, 99.9f, 0.1f);
  EXPECT_FLOAT_EQ(cycles.state, 100.0f);
}

// ── Port voltage ──────────────────────────────────────────────────────────────

TEST(SeplosBmsBleStatusTest, PortVoltage) {
  TestableSeplosBmsBle bms;
  sensor::Sensor port;
  bms.set_port_voltage_sensor(&port);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_NEAR(port.state, 48.00f, 0.01f);
}

// ── Switches ──────────────────────────────────────────────────────────────────

TEST(SeplosBmsBleStatusTest, Switches) {
  TestableSeplosBmsBle bms;
  TestableSwitch discharge_sw, charge_sw, current_limit_sw, heating_sw;
  bms.set_discharging_switch(&discharge_sw);
  bms.set_charging_switch(&charge_sw);
  bms.set_current_limit_switch(&current_limit_sw);
  bms.set_heating_switch(&heating_sw);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_TRUE(discharge_sw.state);
  EXPECT_TRUE(charge_sw.state);
  EXPECT_FALSE(current_limit_sw.state);
  EXPECT_FALSE(heating_sw.state);
}

// ── Alarm events ──────────────────────────────────────────────────────────────

TEST(SeplosBmsBleStatusTest, AlarmEvents) {
  TestableSeplosBmsBle bms;
  sensor::Sensor ev1, ev2, ev3, ev4, ev5, ev6, ev7, ev8;
  bms.set_alarm_event1_bitmask_sensor(&ev1);
  bms.set_alarm_event2_bitmask_sensor(&ev2);
  bms.set_alarm_event3_bitmask_sensor(&ev3);
  bms.set_alarm_event4_bitmask_sensor(&ev4);
  bms.set_alarm_event5_bitmask_sensor(&ev5);
  bms.set_alarm_event6_bitmask_sensor(&ev6);
  bms.set_alarm_event7_bitmask_sensor(&ev7);
  bms.set_alarm_event8_bitmask_sensor(&ev8);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_FLOAT_EQ(ev1.state, 0.0f);
  EXPECT_FLOAT_EQ(ev2.state, 0.0f);
  EXPECT_FLOAT_EQ(ev3.state, 0.0f);
  EXPECT_FLOAT_EQ(ev4.state, 0.0f);
  EXPECT_FLOAT_EQ(ev5.state, 0.0f);
  EXPECT_FLOAT_EQ(ev6.state, 0.0f);
  EXPECT_FLOAT_EQ(ev7.state, 0.0f);
  EXPECT_FLOAT_EQ(ev8.state, 0.0f);
}

TEST(SeplosBmsBleStatusTest, ConsolidatedAlarms) {
  TestableSeplosBmsBle bms;
  text_sensor::TextSensor alarms;
  bms.set_alarms_text_sensor(&alarms);

  bms.decode(SINGLE_MACHINE_FRAME);

  EXPECT_EQ(alarms.state, "No alarms");
}

// ── Null sensors do not crash ─────────────────────────────────────────────────

TEST(SeplosBmsBleSafetyTest, NullSensorsDoNotCrash) {
  TestableSeplosBmsBle bms;

  EXPECT_NO_FATAL_FAILURE(bms.decode(SINGLE_MACHINE_FRAME));
}

}  // namespace esphome::seplos_bms_ble::testing
