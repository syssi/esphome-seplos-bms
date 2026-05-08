#pragma once
#include "esphome/components/seplos_bms_v3_ble_pack/seplos_bms_v3_ble_pack.h"

namespace esphome::seplos_bms_v3_ble_pack::testing {

class TestableSeplosBmsV3BlePack : public SeplosBmsV3BlePack {
 public:
  void setup() override {}
};

}  // namespace esphome::seplos_bms_v3_ble_pack::testing
