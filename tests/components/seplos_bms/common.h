#pragma once
#include "esphome/components/seplos_bms/seplos_bms.h"

namespace esphome::seplos_bms::testing {

class TestableSeplosBms : public SeplosBms {
 public:
  void update() override {}
};

}  // namespace esphome::seplos_bms::testing
