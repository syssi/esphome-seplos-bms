#pragma once
#include "esphome/components/seplos_bms_v3_ble/seplos_bms_v3_ble.h"

namespace esphome::seplos_bms_v3_ble::testing {

class TestableSeplosBmsV3Ble : public SeplosBmsV3Ble {
 public:
  void update() override {}
  void decode_eia(const std::vector<uint8_t> &data) { decode_eia_data_(data); }
  void decode_eib(const std::vector<uint8_t> &data) { decode_eib_data_(data); }
  void decode_eic(const std::vector<uint8_t> &data) { decode_eic_data_(data); }
  void decode_pct(const std::vector<uint8_t> &data) { decode_pct_data_(data); }
  void decode_spa1(const std::vector<uint8_t> &data) { decode_spa1_data_(data); }
  void decode_spa2(const std::vector<uint8_t> &data) { decode_spa2_data_(data); }
};

}  // namespace esphome::seplos_bms_v3_ble::testing
