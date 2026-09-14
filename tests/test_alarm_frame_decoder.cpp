// Standalone C++ unit tests for SEPLOS BMS alarm frame decoding.
// Verifies byte offsets and string formatting against the real protocol frames.
//
// Build and run:
//   g++ -std=c++17 -Wall -o /tmp/test_alarm tests/test_alarm_frame_decoder.cpp && /tmp/test_alarm

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Alarm event names (mirrors seplos_bms.cpp)
// ---------------------------------------------------------------------------

static constexpr const char *const ALARM_EVENT1_NAMES[] = {
    "Voltage sensor fault",       "Temperature sensor fault", "Current sensor fault",   "Key switch fault",
    "Cell voltage dropout fault", "Charge switch fault",      "Discharge switch fault", "Current limit switch fault",
};
static constexpr const char *const ALARM_EVENT2_NAMES[] = {
    "Cell high voltage alarm", "Cell overvoltage protection", "Cell low voltage alarm", "Cell undervoltage protection",
    "Pack high voltage alarm", "Pack overvoltage protection", "Pack low voltage alarm", "Pack undervoltage protection",
};
static constexpr const char *const ALARM_EVENT3_NAMES[] = {
    "Charge high temp alarm",      "Charge overtemp protection",     "Charge low temp alarm",
    "Charge undertemp protection", "Discharge high temp alarm",      "Discharge overtemp protection",
    "Discharge low temp alarm",    "Discharge undertemp protection",
};
static constexpr const char *const ALARM_EVENT4_NAMES[] = {
    "Env high temp alarm",       "Env overtemp protection", "Env low temp alarm",    "Env undertemp protection",
    "Power overtemp protection", "Power high temp alarm",   "Cell low temp heating", "Reserved",
};
static constexpr const char *const ALARM_EVENT5_NAMES[] = {
    "Charge overcurrent alarm",         "Charge overcurrent protection",    "Discharge overcurrent alarm",
    "Discharge overcurrent protection", "Transient overcurrent protection", "Output short circuit protection",
    "Transient overcurrent lockout",    "Output short circuit lockout",
};
static constexpr const char *const ALARM_EVENT6_NAMES[] = {
    "Charge high voltage protection",
    "Intermittent recharge waiting",
    "Residual capacity alarm",
    "Residual capacity protection",
    "Cell low voltage charging prohibition",
    "Output reverse polarity protection",
    "Output connection fault",
    "Inside bit",
};
static constexpr const char *const ALARM_EVENT7_NAMES[] = {
    "Inside bit", "Inside bit", "Inside bit", "Inside bit", "Automatic charging waiting", "Manual charging waiting",
    "Inside bit", "Inside bit",
};
static constexpr const char *const ALARM_EVENT8_NAMES[] = {
    "EEP storage fault",
    "RTC error",
    "Voltage calibration not performed",
    "Current calibration not performed",
    "Zero calibration not performed",
    "Inside bit",
    "Inside bit",
    "Inside bit",
};

// ---------------------------------------------------------------------------
// Pure functions (mirrors seplos_bms.cpp member functions)
// ---------------------------------------------------------------------------

static std::string bitmask_to_string(const char *const messages[], uint8_t messages_size, uint8_t mask) {
  std::string result;
  if (mask) {
    for (uint8_t i = 0; i < messages_size; i++) {
      if (mask & (1 << i)) {
        if (!result.empty())
          result.append("; ");
        result.append(messages[i]);
      }
    }
  }
  return result;
}

static std::string decode_all_alarm_events(uint8_t e1, uint8_t e2, uint8_t e3, uint8_t e4, uint8_t e5, uint8_t e6,
                                           uint8_t e7, uint8_t e8) {
  std::string out;
  auto append = [&](const char *prefix, const char *const names[], uint8_t mask) {
    std::string s = bitmask_to_string(names, 8, mask);
    if (s.empty())
      return;
    if (!out.empty())
      out.append("; ");
    out.append(prefix).append(s);
  };
  append("HW: ", ALARM_EVENT1_NAMES, e1);
  append("VOLT: ", ALARM_EVENT2_NAMES, e2);
  append("TEMP: ", ALARM_EVENT3_NAMES, e3);
  append("ENV: ", ALARM_EVENT4_NAMES, e4);
  append("CURR: ", ALARM_EVENT5_NAMES, e5);
  append("CHG: ", ALARM_EVENT6_NAMES, e6);
  append("SYS: ", ALARM_EVENT7_NAMES, e7);
  append("CAL: ", ALARM_EVENT8_NAMES, e8);
  return out.empty() ? "No alarms" : out;
}

// ---------------------------------------------------------------------------
// Frame offset helper — matches on_alarm_data_ logic
// ---------------------------------------------------------------------------

struct AlarmOffsets {
  size_t base;  // index of alarm_event1
  uint8_t M;
  uint8_t N;
  bool valid;
};

static AlarmOffsets compute_offsets(const std::vector<uint8_t> &data) {
  if (data.size() < 9)
    return {0, 0, 0, false};
  uint8_t M = data[8];
  if (M < 8 || M > 16)
    return {0, 0, 0, false};
  size_t offset_N = 9 + M;
  if (data.size() <= offset_N)
    return {0, 0, 0, false};
  uint8_t N = data[offset_N];
  if (N > 8)
    return {0, 0, 0, false};
  size_t base = 9 + M + 1 + N + 3;
  if (data.size() < base + 14)
    return {0, 0, 0, false};
  return {base, M, N, true};
}

// ---------------------------------------------------------------------------
// Test helpers
// ---------------------------------------------------------------------------

static int g_pass = 0, g_fail = 0;

#define CHECK(cond) \
  do { \
    if (cond) { \
      g_pass++; \
    } else { \
      g_fail++; \
      fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
  } while (0)

#define CHECK_EQ(a, b) \
  do { \
    auto _a = (a); \
    auto _b = (b); \
    if (_a == _b) { \
      g_pass++; \
    } else { \
      g_fail++; \
      fprintf(stderr, "FAIL %s:%d: %s == %s (%s != %s)\n", __FILE__, __LINE__, #a, #b, std::to_string(_a).c_str(), \
              std::to_string(_b).c_str()); \
    } \
  } while (0)

#define CHECK_STR(a, b) \
  do { \
    std::string _a = (a); \
    std::string _b = (b); \
    if (_a == _b) { \
      g_pass++; \
    } else { \
      g_fail++; \
      fprintf(stderr, "FAIL %s:%d: \"%s\" != \"%s\"\n", __FILE__, __LINE__, _a.c_str(), _b.c_str()); \
    } \
  } while (0)

// ---------------------------------------------------------------------------
// Real alarm frame from tests/esp8266-fake-bms.yaml (decoded from ASCII)
// ASCII: ~20004600A06000010F000000000000000000000000000000
//         060000000000000000140000000000000300000200000000000000000002EB74\r
// ---------------------------------------------------------------------------
static const std::vector<uint8_t> REAL_FRAME_NORMAL = {
    0x20, 0x00, 0x46, 0x00, 0xA0, 0x60,              // header: VER ADR CID1 RTN LCHKSUM LENID
    0x00, 0x01,                                      // data[6]=DATA_FLAG data[7]=COMMAND_GROUP=1
    0x0F,                                            // data[8]=M=15 cells
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // data[9..16] cell alarms 1-8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        // data[17..23] cell alarms 9-15
    0x06,                                            // data[24]=N=6 temperatures
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,              // data[25..30] temp alarms 1-6
    0x00,                                            // data[31] charge/discharge current alarm
    0x00,                                            // data[32] total voltage alarm
    0x14,                                            // data[33] P=20 custom alarms
    0x00,                                            // data[34] alarm_event1 (HW)
    0x00,                                            // data[35] alarm_event2 (voltage)
    0x00,                                            // data[36] alarm_event3 (temperature)
    0x00,                                            // data[37] alarm_event4 (env)
    0x00,                                            // data[38] alarm_event5 (current)
    0x00,                                            // data[39] alarm_event6 (output)
    0x03,                                            // data[40] on-off state: discharge+charge ON
    0x00,                                            // data[41] eq_state1 (cells 1-8, no balancing)
    0x00,                                            // data[42] eq_state2 (cells 9-16)
    0x02,                                            // data[43] system_state: charge active
    0x00,                                            // data[44] dis_state1 (cells 1-8)
    0x00,                                            // data[45] dis_state2 (cells 9-16)
    0x00,                                            // data[46] alarm_event7
    0x00,                                            // data[47] alarm_event8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02,              // data[48..53] reservation
};

// Alarm frame with active voltage alarm and cell 3 balancing
static const std::vector<uint8_t> REAL_FRAME_WITH_ALARMS = {
    0x20, 0x00, 0x46, 0x00, 0xA0, 0x60, 0x00, 0x01,
    0x0F,                                            // M=15 cells
    0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,  // cell 3 = 0x02 (upper limit alarm)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x06,  // N=6
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00,  // current alarm: normal
    0x00,  // voltage alarm: normal (byte alarms)
    0x14,  // P=20
    0x00,  // alarm_event1: no HW fault
    0x04,  // alarm_event2: bit2 = monomer low voltage alarm
    0x00,  // alarm_event3
    0x00,  // alarm_event4
    0x00,  // alarm_event5
    0x00,  // alarm_event6
    0x03,  // on-off: discharge+charge ON
    0x04,  // eq_state1: cell 3 balancing (bit2)
    0x00,  // eq_state2
    0x02,  // system_state: charging
    0x00,  // dis_state1
    0x00,  // dis_state2
    0x00,  // alarm_event7
    0x00,  // alarm_event8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
};

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

static void test_bitmask_to_string() {
  // Empty mask → empty string
  CHECK_STR(bitmask_to_string(ALARM_EVENT1_NAMES, 8, 0x00), "");

  // Single bit
  CHECK_STR(bitmask_to_string(ALARM_EVENT1_NAMES, 8, 0x01), "Voltage sensor fault");
  CHECK_STR(bitmask_to_string(ALARM_EVENT1_NAMES, 8, 0x02), "Temperature sensor fault");
  CHECK_STR(bitmask_to_string(ALARM_EVENT1_NAMES, 8, 0x80), "Current limit switch fault");

  // Multiple bits joined with "; "
  CHECK_STR(bitmask_to_string(ALARM_EVENT1_NAMES, 8, 0x03), "Voltage sensor fault; Temperature sensor fault");

  // Voltage event: bit2 = "Cell low voltage alarm"
  CHECK_STR(bitmask_to_string(ALARM_EVENT2_NAMES, 8, 0x04), "Cell low voltage alarm");

  // All bits set for event2
  std::string all_ev2 = bitmask_to_string(ALARM_EVENT2_NAMES, 8, 0xFF);
  CHECK(all_ev2.find("Cell high voltage alarm") != std::string::npos);
  CHECK(all_ev2.find("Pack undervoltage protection") != std::string::npos);
}

static void test_decode_all_alarm_events_no_alarms() {
  CHECK_STR(decode_all_alarm_events(0, 0, 0, 0, 0, 0, 0, 0), "No alarms");
}

static void test_decode_all_alarm_events_with_alarms() {
  // event2 bit2 = voltage, event5 bit0 = current
  std::string result = decode_all_alarm_events(0, 0x04, 0, 0, 0x01, 0, 0, 0);
  CHECK(result.find("VOLT: Cell low voltage alarm") != std::string::npos);
  CHECK(result.find("CURR: Charge overcurrent alarm") != std::string::npos);
  CHECK(result.find("No alarms") == std::string::npos);

  // event8 bit0 = EEP storage fault
  std::string result2 = decode_all_alarm_events(0, 0, 0, 0, 0, 0, 0, 0x01);
  CHECK_STR(result2, "CAL: EEP storage fault");
}

static void test_frame_offsets_normal() {
  auto off = compute_offsets(REAL_FRAME_NORMAL);
  CHECK(off.valid);
  CHECK_EQ(off.M, 15);
  CHECK_EQ(off.N, 6);

  // For M=15, N=6: base = 9+15+1+6+3 = 34
  CHECK_EQ(off.base, size_t(34));

  // All alarm events zero
  CHECK_EQ(REAL_FRAME_NORMAL[off.base + 0], 0x00);  // alarm_event1
  CHECK_EQ(REAL_FRAME_NORMAL[off.base + 1], 0x00);  // alarm_event2 (voltage)
  CHECK_EQ(REAL_FRAME_NORMAL[off.base + 5], 0x00);  // alarm_event6
  CHECK_EQ(REAL_FRAME_NORMAL[off.base + 6], 0x03);  // on-off: discharge+charge ON
  CHECK_EQ(REAL_FRAME_NORMAL[off.base + 7], 0x00);  // eq_state1: no balancing
  CHECK_EQ(REAL_FRAME_NORMAL[off.base + 8], 0x00);  // eq_state2
  CHECK_EQ(REAL_FRAME_NORMAL[off.base + 9], 0x02);  // system_state: charging

  // Switch states
  uint8_t on_off = REAL_FRAME_NORMAL[off.base + 6];
  CHECK((on_off & 0x01) != 0);  // discharge switch ON
  CHECK((on_off & 0x02) != 0);  // charge switch ON

  // Balancing
  uint8_t eq1 = REAL_FRAME_NORMAL[off.base + 7];
  uint8_t eq2 = REAL_FRAME_NORMAL[off.base + 8];
  uint16_t balancing = uint16_t(eq1) | (uint16_t(eq2) << 8);
  CHECK_EQ(balancing, 0x0000);  // no balancing

  // System state: charging bit set
  uint8_t sys = REAL_FRAME_NORMAL[off.base + 9];
  CHECK((sys & 0x02) != 0);  // charge active
  CHECK((sys & 0x01) == 0);  // not discharging
}

static void test_frame_offsets_with_alarms() {
  auto off = compute_offsets(REAL_FRAME_WITH_ALARMS);
  CHECK(off.valid);
  CHECK_EQ(off.base, size_t(34));

  // alarm_event2 bit2: monomer low voltage alarm
  uint8_t ev2 = REAL_FRAME_WITH_ALARMS[off.base + 1];
  CHECK_EQ(ev2, 0x04);
  CHECK((ev2 & 0x04) != 0);  // low voltage alarm
  std::string ev2_str = bitmask_to_string(ALARM_EVENT2_NAMES, 8, ev2);
  CHECK_STR(ev2_str, "Cell low voltage alarm");

  // Cell 3 balancing (eq_state1 bit2)
  uint8_t eq1 = REAL_FRAME_WITH_ALARMS[off.base + 7];
  CHECK_EQ(eq1, 0x04);
  uint16_t balancing = uint16_t(eq1) | (uint16_t(REAL_FRAME_WITH_ALARMS[off.base + 8]) << 8);
  CHECK_EQ(balancing, 0x0004);
  CHECK((balancing & (1 << 2)) != 0);  // cell 3 balancing (0-indexed bit 2)

  // Voltage protection triggered
  CHECK(ev2 != 0);
}

static void test_frame_too_short() {
  std::vector<uint8_t> short_frame(8, 0x00);
  auto off = compute_offsets(short_frame);
  CHECK(!off.valid);

  // Frame with invalid M
  std::vector<uint8_t> bad_m = {0x20, 0x00, 0x46, 0x00, 0xA0, 0x60, 0x00, 0x01, 0x07};
  auto off2 = compute_offsets(bad_m);
  CHECK(!off2.valid);  // M=7 < 8
}

static void test_m16_frame() {
  // M=16, N=6: base = 9+16+1+6+3 = 35
  std::vector<uint8_t> frame16(55, 0x00);
  frame16[0] = 0x20;
  frame16[2] = 0x46;
  frame16[8] = 16;                   // M=16
  frame16[9 + 16] = 6;               // N=6  at data[25]
  frame16[9 + 16 + 1 + 6 + 2] = 20;  // P=20
  frame16[35] = 0x01;                // alarm_event1: voltage sensor fault
  frame16[41] = 0x03;                // on-off: discharge+charge ON

  auto off = compute_offsets(frame16);
  CHECK(off.valid);
  CHECK_EQ(off.M, 16);
  CHECK_EQ(off.N, 6);
  CHECK_EQ(off.base, size_t(35));
  CHECK_EQ(frame16[off.base + 0], 0x01);  // alarm_event1
  CHECK_EQ(frame16[off.base + 6], 0x03);  // on-off state
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main() {
  test_bitmask_to_string();
  test_decode_all_alarm_events_no_alarms();
  test_decode_all_alarm_events_with_alarms();
  test_frame_offsets_normal();
  test_frame_offsets_with_alarms();
  test_frame_too_short();
  test_m16_frame();

  printf("%d passed, %d failed\n", g_pass, g_fail);
  return g_fail == 0 ? 0 : 1;
}
