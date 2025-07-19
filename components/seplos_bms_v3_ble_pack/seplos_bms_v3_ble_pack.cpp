#include "seplos_bms_v3_ble_pack.h"
#include "esphome/core/log.h"

namespace esphome {
namespace seplos_bms_v3_ble_pack {

static const char *const TAG = "seplos_bms_v3_ble_pack";

void SeplosBmsV3BlePack::setup() { ESP_LOGCONFIG(TAG, "Setting up Pack Sensor 0x%02X", this->address_); }

void SeplosBmsV3BlePack::dump_config() {
  ESP_LOGCONFIG(TAG, "Pack Sensor:");
  ESP_LOGCONFIG(TAG, "  Pack Address: 0x%02X", this->address_);

  if (this->pack_voltage_sensor_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Pack Voltage Sensor configured");
  }
  if (this->pack_current_sensor_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Pack Current Sensor configured");
  }
  if (this->pack_battery_level_sensor_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Pack Battery Level Sensor configured");
  }
  if (this->pack_cycle_sensor_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Pack Cycle Sensor configured");
  }

  // Count configured cell sensors
  int cell_count = 0;
  for (auto &pack_cell_voltage_sensor : this->pack_cell_voltage_sensors_) {
    if (pack_cell_voltage_sensor != nullptr) {
      cell_count++;
    }
  }
  if (cell_count > 0) {
    ESP_LOGCONFIG(TAG, "  Pack Cell Voltage Sensors: %d configured", cell_count);
  }

  // Count configured temperature sensors
  int temp_count = 0;
  for (auto &pack_temperature_sensor : this->pack_temperature_sensors_) {
    if (pack_temperature_sensor != nullptr) {
      temp_count++;
    }
  }
  if (temp_count > 0) {
    ESP_LOGCONFIG(TAG, "  Pack Temperature Sensors: %d configured", temp_count);
  }
}

void SeplosBmsV3BlePack::update_pack_voltage(float voltage) {
  if (this->pack_voltage_sensor_ != nullptr) {
    this->pack_voltage_sensor_->publish_state(voltage);
  }
}

void SeplosBmsV3BlePack::update_pack_current(float current) {
  if (this->pack_current_sensor_ != nullptr) {
    this->pack_current_sensor_->publish_state(current);
  }
}

void SeplosBmsV3BlePack::update_pack_battery_level(float level) {
  if (this->pack_battery_level_sensor_ != nullptr) {
    this->pack_battery_level_sensor_->publish_state(level);
  }
}

void SeplosBmsV3BlePack::update_pack_cycle(float cycle) {
  if (this->pack_cycle_sensor_ != nullptr) {
    this->pack_cycle_sensor_->publish_state(cycle);
  }
}

void SeplosBmsV3BlePack::update_pack_cell_voltage(uint8_t index, float voltage) {
  if (index < 32 && this->pack_cell_voltage_sensors_[index] != nullptr) {
    this->pack_cell_voltage_sensors_[index]->publish_state(voltage);
  }
}

void SeplosBmsV3BlePack::update_pack_temperature(uint8_t index, float temperature) {
  if (index < 16 && this->pack_temperature_sensors_[index] != nullptr) {
    this->pack_temperature_sensors_[index]->publish_state(temperature);
  }
}

void SeplosBmsV3BlePack::on_pack_pia_data(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGD(TAG, "Decoding PIA data for pack 0x%02X (%d bytes)", this->address_, data.size());

  if (data.size() < 34) {
    ESP_LOGW(TAG, "PIA data too short: %d bytes", data.size());
    return;
  }

  // Pack voltage
  this->update_pack_voltage(seplos_get_16bit(0) * 0.01f);

  // Pack current
  this->update_pack_current((int16_t) seplos_get_16bit(2) * 0.01f);

  // Pack battery level
  this->update_pack_battery_level(seplos_get_16bit(10) * 0.1f);

  // Pack cycles
  this->update_pack_cycle((float) seplos_get_16bit(14));
}

void SeplosBmsV3BlePack::on_pack_pib_data(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGD(TAG, "Decoding PIB data for pack 0x%02X (%d bytes)", this->address_, data.size());

  if (data.size() < 52) {
    ESP_LOGW(TAG, "PIB data too short: %d bytes", data.size());
    return;
  }

  // Cell voltages (0-31, 16 cells * 2 bytes each)
  for (int i = 0; i < 16; i++) {
    this->update_pack_cell_voltage(i, seplos_get_16bit(i * 2) * 0.001f);
  }

  // Temperature values (32-39, 4 sensors * 2 bytes each)
  for (int i = 0; i < 4; i++) {
    this->update_pack_temperature(i, (seplos_get_16bit(32 + i * 2) - 2731.5f) * 0.1f);
  }
}

void SeplosBmsV3BlePack::on_pack_pic_data(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Decoding PIC data for pack 0x%02X (%d bytes)", this->address_, data.size());

  if (data.size() < 8) {
    ESP_LOGW(TAG, "PIC data too short: %d bytes", data.size());
    return;
  }

  // PIC data contains pack-specific status codes and protection information
  // Based on the protocol, PIC data contains:
  // Byte 0: System state code
  // Byte 1: Voltage event code
  // Byte 2: Temperature event code
  // Byte 4: Current event code

  uint8_t system_state = data[0];
  uint8_t voltage_event = data[1];
  uint8_t temperature_event = data[2];
  uint8_t current_event = data[4];

  ESP_LOGD(TAG, "Pack 0x%02X status - System: 0x%02X, Voltage: 0x%02X, Temperature: 0x%02X, Current: 0x%02X",
           this->address_, system_state, voltage_event, temperature_event, current_event);

  // Log any active alarms/protections
  if (voltage_event != 0) {
    ESP_LOGW(TAG, "Pack 0x%02X voltage protection active: 0x%02X", this->address_, voltage_event);
  }
  if (temperature_event != 0) {
    ESP_LOGW(TAG, "Pack 0x%02X temperature protection active: 0x%02X", this->address_, temperature_event);
  }
  if (current_event != 0) {
    ESP_LOGW(TAG, "Pack 0x%02X current protection active: 0x%02X", this->address_, current_event);
  }
  if (system_state & 0x20) {  // Bit 5 = Turn off state
    ESP_LOGW(TAG, "Pack 0x%02X system fault - turn off state active", this->address_);
  }
}

}  // namespace seplos_bms_v3_ble_pack
}  // namespace esphome
