#pragma once

#include "../seplos_bms_ble.h"
#include "esphome/components/switch/switch.h"

namespace esphome::seplos_bms_ble {

class SeplosBmsBle;
class SeplosSwitch : public switch_::Switch {
 public:
  void set_parent(SeplosBmsBle *parent) { this->parent_ = parent; };
  void set_holding_register(uint8_t payload) { this->holding_register_ = payload; };

 protected:
  void write_state(bool state) override;
  SeplosBmsBle *parent_;
  uint8_t holding_register_;
};

}  // namespace esphome::seplos_bms_ble
