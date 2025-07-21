#include "seplos_bms_v3_ble_pack.h"
#include "esphome/core/log.h"

namespace esphome {
namespace seplos_bms_v3_ble_pack {

static const char *const TAG = "seplos_bms_v3_ble_pack";

void SeplosBmsV3BlePack::setup() { ESP_LOGCONFIG(TAG, "Setting up Pack Sensor 0x%02X", this->get_address()); }

void SeplosBmsV3BlePack::dump_config() {
  ESP_LOGCONFIG(TAG, "Pack Sensor:");
  ESP_LOGCONFIG(TAG, "  Pack Address: 0x%02X", this->get_address());

  LOG_SENSOR("", "Pack Voltage", this->pack_voltage_sensor_);
  LOG_SENSOR("", "Pack Current", this->pack_current_sensor_);
  LOG_SENSOR("", "Pack Battery Level", this->pack_battery_level_sensor_);
  LOG_SENSOR("", "Pack Cycle", this->pack_cycle_sensor_);

  LOG_SENSOR("", "Cell Voltage 1", this->pack_cell_voltage_sensors_[0]);
  LOG_SENSOR("", "Cell Voltage 2", this->pack_cell_voltage_sensors_[1]);
  LOG_SENSOR("", "Cell Voltage 3", this->pack_cell_voltage_sensors_[2]);
  LOG_SENSOR("", "Cell Voltage 4", this->pack_cell_voltage_sensors_[3]);
  LOG_SENSOR("", "Cell Voltage 5", this->pack_cell_voltage_sensors_[4]);
  LOG_SENSOR("", "Cell Voltage 6", this->pack_cell_voltage_sensors_[5]);
  LOG_SENSOR("", "Cell Voltage 7", this->pack_cell_voltage_sensors_[6]);
  LOG_SENSOR("", "Cell Voltage 8", this->pack_cell_voltage_sensors_[7]);
  LOG_SENSOR("", "Cell Voltage 9", this->pack_cell_voltage_sensors_[8]);
  LOG_SENSOR("", "Cell Voltage 10", this->pack_cell_voltage_sensors_[9]);
  LOG_SENSOR("", "Cell Voltage 11", this->pack_cell_voltage_sensors_[10]);
  LOG_SENSOR("", "Cell Voltage 12", this->pack_cell_voltage_sensors_[11]);
  LOG_SENSOR("", "Cell Voltage 13", this->pack_cell_voltage_sensors_[12]);
  LOG_SENSOR("", "Cell Voltage 14", this->pack_cell_voltage_sensors_[13]);
  LOG_SENSOR("", "Cell Voltage 15", this->pack_cell_voltage_sensors_[14]);
  LOG_SENSOR("", "Cell Voltage 16", this->pack_cell_voltage_sensors_[15]);

  LOG_SENSOR("", "Pack Temperature 1", this->pack_temperature_sensors_[0]);
  LOG_SENSOR("", "Pack Temperature 2", this->pack_temperature_sensors_[1]);
  LOG_SENSOR("", "Pack Temperature 3", this->pack_temperature_sensors_[2]);
  LOG_SENSOR("", "Pack Temperature 4", this->pack_temperature_sensors_[3]);

  LOG_SENSOR("", "Ambient Temperature", this->ambient_temperature_sensor_);
  LOG_SENSOR("", "Mosfet Temperature", this->mosfet_temperature_sensor_);
}

void SeplosBmsV3BlePack::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor != nullptr) {
    sensor->publish_state(value);
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

  this->publish_state_(this->pack_voltage_sensor_, seplos_get_16bit(0) * 0.01f);
  this->publish_state_(this->pack_current_sensor_, (int16_t) seplos_get_16bit(2) * 0.01f);
  this->publish_state_(this->pack_battery_level_sensor_, seplos_get_16bit(10) * 0.1f);
  this->publish_state_(this->pack_cycle_sensor_, (float) seplos_get_16bit(14));
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
    this->publish_state_(this->pack_cell_voltage_sensors_[i], voltage);
  }

  // Cell temperatures (32-39, 4 sensors * 2 bytes each)
  for (uint8_t i = 0; i < 4; i++) {
    uint16_t temperature_raw = seplos_get_16bit(32 + i * 2);
    float temperature_celsius = (temperature_raw - 2731.5f) * 0.1f;
    ESP_LOGD(TAG, "  Cell %d temperature: %d (%.1f °C)", i + 1, temperature_raw, temperature_celsius);
    this->publish_state_(this->pack_temperature_sensors_[i], temperature_celsius);
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
