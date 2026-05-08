#pragma once
#include "esphome/components/seplos_bms_ble/seplos_bms_ble.h"
#include "esphome/components/switch/switch.h"

namespace esphome::seplos_bms_ble::testing {

class TestableSeplosBmsBle : public SeplosBmsBle {
 public:
  void update() override {}
  bool send_command(uint8_t function, const std::vector<uint8_t> &payload = {}) { return false; }
};

class TestableSwitch : public switch_::Switch {
 public:
  void write_state(bool state) override { this->publish_state(state); }
};

}  // namespace esphome::seplos_bms_ble::testing
