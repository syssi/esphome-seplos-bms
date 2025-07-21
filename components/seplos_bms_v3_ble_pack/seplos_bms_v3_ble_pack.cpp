#include "seplos_bms_v3_ble_pack.h"
#include "esphome/core/log.h"

namespace esphome {
namespace seplos_bms_v3_ble_pack {

static const char *const TAG = "seplos_bms_v3_ble_pack";

void SeplosBmsV3BlePack::setup() { ESP_LOGCONFIG(TAG, "Setting up Pack Sensor 0x%02X", this->get_address()); }

void SeplosBmsV3BlePack::dump_config() {
  ESP_LOGCONFIG(TAG, "Pack Sensor:");
  ESP_LOGCONFIG(TAG, "  Pack Address: 0x%02X", this->get_address());
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
  if (index < 16 && this->pack_cell_voltage_sensors_[index] != nullptr) {
    this->pack_cell_voltage_sensors_[index]->publish_state(voltage);
  }
}

void SeplosBmsV3BlePack::update_pack_temperature(uint8_t index, float temperature) {
  if (index < 4 && this->pack_temperature_sensors_[index] != nullptr) {
    this->pack_temperature_sensors_[index]->publish_state(temperature);
  }
}

void SeplosBmsV3BlePack::on_pack_pia_data(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGD(TAG, "Decoding PIA data for pack 0x%02X (%d bytes)", this->get_address(), data.size());

  if (data.size() < 34) {
    ESP_LOGW(TAG, "PIA data too short: %d bytes", data.size());
    return;
  }

  this->update_pack_voltage(seplos_get_16bit(0) * 0.01f);
  this->update_pack_current((int16_t) seplos_get_16bit(2) * 0.01f);
  this->update_pack_battery_level(seplos_get_16bit(10) * 0.1f);
  this->update_pack_cycle((float) seplos_get_16bit(14));
}

void SeplosBmsV3BlePack::on_pack_pib_data(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGD(TAG, "Decoding PIB data for pack 0x%02X (%d bytes)", this->get_address(), data.size());

  if (data.size() < 52) {
    ESP_LOGW(TAG, "PIB data too short: %d bytes", data.size());
    return;
  }

  // Cell voltages (0-31, 16 cells * 2 bytes each)
  for (uint8_t i = 0; i < 16; i++) {
    uint16_t voltage_raw = seplos_get_16bit(i * 2);
    float voltage = voltage_raw * 0.001f;
    ESP_LOGD(TAG, "  Cell %d voltage: %d mV (%.3f V)", i + 1, voltage_raw, voltage);
    this->update_pack_cell_voltage(i, voltage);
  }

  // Cell temperatures (32-39, 4 sensors * 2 bytes each)
  for (uint8_t i = 0; i < 4; i++) {
    uint16_t temperature_raw = seplos_get_16bit(32 + i * 2);
    float temperature_celsius = (temperature_raw - 2731.5f) * 0.1f;
    ESP_LOGD(TAG, "  Cell %d temperature: %d (%.1f °C)", i + 1, temperature_raw, temperature_celsius);
    this->update_pack_temperature(i, temperature_celsius);
  }

  // Environment temperature (bytes 40-41) if available
  if (data.size() >= 42) {
    uint16_t env_temperature_raw = seplos_get_16bit(40);
    float env_temperature_celsius = (env_temperature_raw - 2731.5f) * 0.1f;
    ESP_LOGD(TAG, "  Environment temperature: %d (%.1f °C)", env_temperature_raw, env_temperature_celsius);
    // TODO: Add environment temperature sensor when available
  }
}

void SeplosBmsV3BlePack::on_pack_pic_data(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Decoding PIC data for pack 0x%02X (%d bytes)", this->get_address(), data.size());

  if (data.size() < 288) {  // 0x90 * 2 = 288 bytes
    ESP_LOGW(TAG, "PIC data too short: %d bytes (expected 288)", data.size());
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
           this->get_address(), system_state, voltage_event, temperature_event, current_event);

  // Log any active alarms/protections
  if (voltage_event != 0) {
    ESP_LOGW(TAG, "Pack 0x%02X voltage protection active: 0x%02X", this->get_address(), voltage_event);
  }
  if (temperature_event != 0) {
    ESP_LOGW(TAG, "Pack 0x%02X temperature protection active: 0x%02X", this->get_address(), temperature_event);
  }
  if (current_event != 0) {
    ESP_LOGW(TAG, "Pack 0x%02X current protection active: 0x%02X", this->get_address(), current_event);
  }
  if (system_state & 0x20) {  // Bit 5 = Turn off state
    ESP_LOGW(TAG, "Pack 0x%02X system fault - turn off state active", this->get_address());
  }
}

}  // namespace seplos_bms_v3_ble_pack
}  // namespace esphome
