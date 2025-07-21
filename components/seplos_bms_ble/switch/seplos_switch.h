#pragma once

#include "../seplos_bms_ble.h"
#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace seplos_bms_ble {

class SeplosBmsBle;
class SeplosSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(SeplosBmsBle *parent) { this->parent_ = parent; };
  void set_holding_register(uint8_t payload) { this->holding_register_ = payload; };
  void dump_config() override;
  void loop() override {}
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void write_state(bool state) override;
  SeplosBmsBle *parent_;
  uint8_t holding_register_;
};

}  // namespace seplos_bms_ble
}  // namespace esphome
