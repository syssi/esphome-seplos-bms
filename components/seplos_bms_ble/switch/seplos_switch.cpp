#include "seplos_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace seplos_bms_ble {

static const char *const TAG = "seplos_bms_ble.switch";

void SeplosSwitch::dump_config() {
  LOG_SWITCH("", "SeplosBmsBle Switch", this);
  ESP_LOGCONFIG(TAG, "  Holding register: 0x%02X", this->holding_register_);
}

void SeplosSwitch::write_state(bool state) {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "Cannot control switch - parent not set");
    return;
  }

  ESP_LOGI(TAG, "Switch (holding register: 0x%02X) %s requested", this->holding_register_, state ? "ON" : "OFF");

  // Build payload for SEPLOS SET_MOSFET_CONTROL command (0xAA)
  std::vector<uint8_t> payload;
  payload.push_back(this->holding_register_);  // Switch type bitmask
  payload.push_back(state ? 0x01 : 0x00);      // Switch state (1=ON, 0=OFF)

  if (this->parent_->send_command(0xAA, payload)) {
    this->publish_state(state);
    ESP_LOGD(TAG, "Switch control command sent successfully");
  } else {
    ESP_LOGW(TAG, "Failed to send switch control command");
  }
}

}  // namespace seplos_bms_ble
}  // namespace esphome
