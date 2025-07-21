#include "seplos_bms_v3_ble_pack.h"
#include "esphome/core/log.h"

namespace esphome {
namespace seplos_bms_v3_ble_pack {

static const char *const TAG = "seplos_bms_v3_ble_pack";

static const uint16_t SEPLOS_V3_PIA_LENGTH = 0x11;
static const uint16_t SEPLOS_V3_PIB_LENGTH = 0x1A;
static const uint16_t SEPLOS_V3_PIC_LENGTH = 0x90;

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

void SeplosBmsV3BlePack::on_frame_data(const std::vector<uint8_t> &frame) {
  uint16_t data_len = frame[2];
  std::vector<uint8_t> payload(frame.begin() + 3, frame.end() - 2);

  ESP_LOGD(TAG, "Received frame for pack 0x%02X: function=0x%02X, length=%d", frame[0], frame[1], data_len);

  switch (data_len) {
    case SEPLOS_V3_PIA_LENGTH * 2:
      this->decode_pack_pia_data_(payload);
      break;
    case SEPLOS_V3_PIB_LENGTH * 2:
      this->decode_pack_pib_data_(payload);
      break;
    case SEPLOS_V3_PIC_LENGTH * 2:
      this->decode_pack_pic_data_(payload);
      break;
    default:
      ESP_LOGW(TAG, "Unknown pack frame type for pack 0x%02X: length=%d", frame[0], data_len);
      break;
  }
}

void SeplosBmsV3BlePack::decode_pack_pia_data_(const std::vector<uint8_t> &data) {
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

void SeplosBmsV3BlePack::decode_pack_pib_data_(const std::vector<uint8_t> &data) {
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
    this->publish_state_(this->pack_cell_voltage_sensors_[i], seplos_get_16bit(i * 2) * 0.001f);
  }

  // Cell temperatures (32-39, 4 sensors * 2 bytes each)
  for (uint8_t i = 0; i < 4; i++) {
    this->publish_state_(this->pack_temperature_sensors_[i], (seplos_get_16bit(32 + i * 2) - 2731.5f) * 0.1f);
  }

  // Environment temperature (bytes 40-41) if available
  if (data.size() >= 42) {
    uint16_t env_temperature_raw = seplos_get_16bit(40);
    float env_temperature_celsius = (env_temperature_raw - 2731.5f) * 0.1f;
    ESP_LOGD(TAG, "  Environment temperature: %d (%.1f °C)", env_temperature_raw, env_temperature_celsius);
    // TODO: Add environment temperature sensor when available
  }
}

void SeplosBmsV3BlePack::decode_pack_pic_data_(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Decoding PIC data for pack 0x%02X (%d bytes)", this->get_address(), data.size());

  if (data.size() < 288) {  // 0x90 * 2 = 288 bytes
    ESP_LOGW(TAG, "PIC data too short: %d bytes (expected 288)", data.size());
    return;
  }

  uint8_t system_state = data[0];
  ESP_LOGD(TAG, "System State (TB09) - Pack 0x%02X: 0x%02X", this->get_address(), system_state);
  if (system_state & 0x01)
    ESP_LOGD(TAG, "  - Discharging active");
  if (system_state & 0x02)
    ESP_LOGD(TAG, "  - Charging active");
  if (system_state & 0x04)
    ESP_LOGD(TAG, "  - Floating charge");
  if (system_state & 0x08)
    ESP_LOGD(TAG, "  - Full charge");
  if (system_state & 0x10)
    ESP_LOGD(TAG, "  - Standby mode");
  if (system_state & 0x20)
    ESP_LOGW(TAG, "  - System turned off");

  uint8_t voltage_event = data[1];
  ESP_LOGD(TAG, "Voltage Events (TB02) - Pack 0x%02X: 0x%02X", this->get_address(), voltage_event);
  if (voltage_event & 0x01)
    ESP_LOGW(TAG, "  - Cell high voltage alarm");
  if (voltage_event & 0x02)
    ESP_LOGE(TAG, "  - Cell overvoltage protection!");
  if (voltage_event & 0x04)
    ESP_LOGW(TAG, "  - Cell low voltage alarm");
  if (voltage_event & 0x08)
    ESP_LOGE(TAG, "  - Cell undervoltage protection!");
  if (voltage_event & 0x10)
    ESP_LOGW(TAG, "  - Pack high voltage alarm");
  if (voltage_event & 0x20)
    ESP_LOGE(TAG, "  - Pack overvoltage protection!");
  if (voltage_event & 0x40)
    ESP_LOGW(TAG, "  - Pack low voltage alarm");
  if (voltage_event & 0x80)
    ESP_LOGE(TAG, "  - Pack undervoltage protection!");

  uint8_t temperature_event = data[2];
  ESP_LOGD(TAG, "Temperature Events (TB03) - Pack 0x%02X: 0x%02X", this->get_address(), temperature_event);
  if (temperature_event & 0x01)
    ESP_LOGW(TAG, "  - Charge high temperature alarm");
  if (temperature_event & 0x02)
    ESP_LOGE(TAG, "  - Charge overtemperature protection!");
  if (temperature_event & 0x04)
    ESP_LOGW(TAG, "  - Charge low temperature alarm");
  if (temperature_event & 0x08)
    ESP_LOGE(TAG, "  - Charge undertemperature protection!");
  if (temperature_event & 0x10)
    ESP_LOGW(TAG, "  - Discharge high temperature alarm");
  if (temperature_event & 0x20)
    ESP_LOGE(TAG, "  - Discharge overtemperature protection!");
  if (temperature_event & 0x40)
    ESP_LOGW(TAG, "  - Discharge low temperature alarm");
  if (temperature_event & 0x80)
    ESP_LOGE(TAG, "  - Discharge undertemperature protection!");

  uint8_t current_event = data[4];
  ESP_LOGD(TAG, "Current Events (TB05) - Pack 0x%02X: 0x%02X", this->get_address(), current_event);
  if (current_event & 0x01)
    ESP_LOGW(TAG, "  - Charge current alarm");
  if (current_event & 0x02)
    ESP_LOGE(TAG, "  - Charge overcurrent protection!");
  if (current_event & 0x04)
    ESP_LOGE(TAG, "  - Charge secondary overcurrent protection!");
  if (current_event & 0x08)
    ESP_LOGW(TAG, "  - Discharge current alarm");
  if (current_event & 0x10)
    ESP_LOGE(TAG, "  - Discharge overcurrent protection!");
  if (current_event & 0x20)
    ESP_LOGE(TAG, "  - Discharge secondary overcurrent protection!");
  if (current_event & 0x40)
    ESP_LOGE(TAG, "  - Short circuit protection!");
}

}  // namespace seplos_bms_v3_ble_pack
}  // namespace esphome
