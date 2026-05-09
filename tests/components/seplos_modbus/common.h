#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include "esphome/components/seplos_modbus/seplos_modbus.h"

namespace esphome::seplos_modbus::testing {

static std::vector<uint8_t> make_seplos_frame(const std::string &ascii_payload) {
  uint16_t s = 0;
  for (char c : ascii_payload)
    s += static_cast<uint8_t>(c);
  s = (~s) + 1;
  char crc[5];
  snprintf(crc, sizeof(crc), "%04X", s);
  std::vector<uint8_t> frame;
  frame.push_back(0x7E);
  for (char c : ascii_payload)
    frame.push_back(static_cast<uint8_t>(c));
  for (int i = 0; i < 4; i++)
    frame.push_back(static_cast<uint8_t>(crc[i]));
  frame.push_back(0x0D);
  return frame;
}

// VER=0x20, ADDR=0x00, CID1=0x46, CID2=0x00 -> decoded address=0x00
static const std::vector<uint8_t> FRAME_ADDR_00 = make_seplos_frame("20004600");
// VER=0x20, ADDR=0x01, CID1=0x46, CID2=0x00 -> decoded address=0x01
static const std::vector<uint8_t> FRAME_ADDR_01 = make_seplos_frame("20014600");

class MockSeplosModbusDevice : public SeplosModbusDevice {
 public:
  std::vector<uint8_t> received_data;
  int call_count{0};

  void on_seplos_modbus_data(const std::vector<uint8_t> &data) override {
    received_data = data;
    call_count++;
  }
};

class TestableSeplosModbus : public SeplosModbus {
 public:
  void loop() override {}
  using SeplosModbus::parse_seplos_modbus_byte_;

  bool feed(const std::vector<uint8_t> &frame) {
    bool result = false;
    for (uint8_t byte : frame)
      result = parse_seplos_modbus_byte_(byte);
    return result;
  }
};

}  // namespace esphome::seplos_modbus::testing
