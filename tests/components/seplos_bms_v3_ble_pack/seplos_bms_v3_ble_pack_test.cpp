#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

namespace esphome::seplos_bms_v3_ble_pack::testing {

// ── Pack voltage, current, battery level, cycle ───────────────────────────────

TEST(SeplosBmsV3BlePackStatusTest, PackVoltageCurrentBatteryLevelCycle) {
  TestableSeplosBmsV3BlePack pack;
  pack.set_address(0x01);
  sensor::Sensor voltage, current, battery_level, cycle;
  pack.set_pack_voltage_sensor(&voltage);
  pack.set_pack_current_sensor(&current);
  pack.set_pack_battery_level_sensor(&battery_level);
  pack.set_pack_cycle_sensor(&cycle);

  pack.on_frame_data(PACK_PIA_FRAME);

  EXPECT_NEAR(voltage.state, 52.80f, 0.01f);
  EXPECT_NEAR(current.state, -5.00f, 0.01f);
  EXPECT_NEAR(battery_level.state, 75.5f, 0.1f);
  EXPECT_FLOAT_EQ(cycle.state, 25.0f);
}

// ── Cell voltages ─────────────────────────────────────────────────────────────

TEST(SeplosBmsV3BlePackStatusTest, CellVoltages) {
  TestableSeplosBmsV3BlePack pack;
  pack.set_address(0x01);
  sensor::Sensor cells[16];
  for (int i = 0; i < 16; i++)
    pack.set_pack_cell_voltage_sensor(i, &cells[i]);

  pack.on_frame_data(PACK_PIB_FRAME);

  EXPECT_NEAR(cells[0].state, 3.310f, 0.001f);
  EXPECT_NEAR(cells[1].state, 3.300f, 0.001f);
  EXPECT_NEAR(cells[2].state, 3.290f, 0.001f);
  EXPECT_NEAR(cells[3].state, 3.320f, 0.001f);
  for (int i = 4; i < 16; i++)
    EXPECT_NEAR(cells[i].state, 3.300f, 0.001f);
}

// ── Cell temperatures ─────────────────────────────────────────────────────────

TEST(SeplosBmsV3BlePackStatusTest, CellTemperatures) {
  TestableSeplosBmsV3BlePack pack;
  pack.set_address(0x01);
  sensor::Sensor t0, t1, t2, t3;
  pack.set_pack_temperature_sensor(0, &t0);
  pack.set_pack_temperature_sensor(1, &t1);
  pack.set_pack_temperature_sensor(2, &t2);
  pack.set_pack_temperature_sensor(3, &t3);

  pack.on_frame_data(PACK_PIB_FRAME);

  EXPECT_NEAR(t0.state, 25.05f, 0.1f);
  EXPECT_NEAR(t1.state, 29.95f, 0.1f);
  EXPECT_NEAR(t2.state, 20.05f, 0.1f);
  EXPECT_NEAR(t3.state, 15.05f, 0.1f);
}

TEST(SeplosBmsV3BlePackStatusTest, AmbientAndMosfetTemperatures) {
  TestableSeplosBmsV3BlePack pack;
  pack.set_address(0x01);
  sensor::Sensor ambient, mosfet;
  pack.set_ambient_temperature_sensor(&ambient);
  pack.set_mosfet_temperature_sensor(&mosfet);

  pack.on_frame_data(PACK_PIB_FRAME);

  // Read from registers 0x1118/0x1119 (bytes 48/50), not the 0x1114-0x1117 reserve
  EXPECT_NEAR(ambient.state, 23.45f, 0.1f);
  EXPECT_NEAR(mosfet.state, 22.15f, 0.1f);
}

// ── Null sensors do not crash ─────────────────────────────────────────────────

TEST(SeplosBmsV3BlePackSafetyTest, NullSensorsDoNotCrash) {
  TestableSeplosBmsV3BlePack pack;
  pack.set_address(0x01);

  EXPECT_NO_FATAL_FAILURE(pack.on_frame_data(PACK_PIA_FRAME));
  EXPECT_NO_FATAL_FAILURE(pack.on_frame_data(PACK_PIB_FRAME));
}

}  // namespace esphome::seplos_bms_v3_ble_pack::testing
