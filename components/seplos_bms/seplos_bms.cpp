#include "seplos_bms.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace seplos_bms {

static const char *const TAG = "seplos_bms";

void SeplosBms::on_seplos_modbus_data(const std::vector<uint8_t> &data) {
  if (data.size() == 81 && data[5] == 0x96) {
    this->on_telemetry_data(data);
    return;
  }

  ESP_LOGW(TAG, "Unhandled data received: %s", format_hex_pretty(&data.front(), data.size()).c_str());
}

void SeplosBms::on_telemetry_data(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGI(TAG, "Telemetry frame received");

  // ->
  // 0x2000460010960001100CD70CE90CF40CD60CEF0CE50CE10CDC0CE90CF00CE80CEF0CEA0CDA0CDE0CD8060BA60BA00B970BA60BA50BA2FD5C14A0344E0A426803134650004603E8149F0000000000000000
  //
  // *Data*
  //
  // Byte   Address Content: Description                      Decoded content               Coeff./Unit
  //   0    0x20             Protocol version      VER        2.0
  //   1    0x00             Device address        ADR
  //   2    0x46             Device type           CID1       Lithium iron phosphate battery BMS
  //   3    0x00             Function code         CID2       0x00: Normal, 0x01 VER error, 0x02 Chksum error, ...
  //   4    0x10             Data length checksum  LCHKSUM
  //   5    0x96             Data length           LENID      150 / 2 = 75
  //   6      0x00           Data flag
  //   7      0x01           Command group
  //   8      0x10           Number of cells                  16
  ESP_LOGD(TAG, "Number of cells: %d", data[8]);
  //   9      0x0C 0xD7      Cell voltage 1                   3287 * 0.001f = 3.287         V
  //   11     0x0C 0xE9      Cell voltage 2                   3305 * 0.001f = 3.305         V
  //   ...    ...            ...
  //   39     0x0C 0xD8      Cell voltage 16                                                V
  for (uint8_t i = 0; i < data[8]; i++) {
    ESP_LOGD(TAG, "Cell voltage %d: %.3f V", i + 1, (float) seplos_get_16bit(9 + (i * 2)) * 0.001f);
  }

  //   41     0x06           Number of temperatures           6                             V
  ESP_LOGD(TAG, "Number of temperature sensors: %d", data[41]);

  //   42     0x0B 0xA6      Temperature sensor 1             2982 * 0.01f = 29.82          °C
  //   44     0x0B 0xA0      Temperature sensor 2             2976 * 0.01f = 29.76          °C
  //   46     0x0B 0x97      Temperature sensor 3             2967 * 0.01f = 29.67          °C
  //   48     0x0B 0xA6      Temperature sensor 4             2982 * 0.01f = 29.82          °C
  for (uint8_t i = 0; i < data[41] - 2; i++) {
    ESP_LOGD(TAG, "Temperature sensor %d: %.2f °C", i, (float) seplos_get_16bit(42 + (i * 2)) * 0.01f);
  }

  //   50     0x0B 0xA5      Environment temperature          2981 * 0.01f = 29.81          °C
  ESP_LOGD(TAG, "Environment temperature: %.2f °C", (float) seplos_get_16bit(50) * 0.01f);

  //   52     0x0B 0xA2      Mosfet temperature               2978 * 0.01f = 29.78          °C
  ESP_LOGD(TAG, "Mosfet temperature: %.2f °C", (float) seplos_get_16bit(52) * 0.01f);

  //   54     0xFD 0x5C      Charge/discharge current         signed int?                   A
  ESP_LOGD(TAG, "Current: %.2f A", (float) ((int16_t) seplos_get_16bit(54)) * 0.01f);

  //   56     0x14 0xA0      Total battery voltage            5280 * 0.01f = 52.80          V
  ESP_LOGD(TAG, "Total battery voltage: %.2f V", (float) seplos_get_16bit(56) * 0.01f);

  //   58     0x34 0x4E      Residual capacity                13390 * 0.01f = 133.90        Ah
  ESP_LOGD(TAG, "Residual capacity: %.2f Ah", (float) seplos_get_16bit(58) * 0.01f);

  //   60     0x0A           Custom number                    10
  //   61     0x42 0x68      Battery capacity                 17000 * 0.01f = 170.00        Ah
  ESP_LOGD(TAG, "Battery capacity: %.2f Ah", (float) seplos_get_16bit(61) * 0.01f);

  //   63     0x03 0x13      Stage of charge                  787 * 0.1f = 78.7             %
  ESP_LOGD(TAG, "State of charge: %.1f %%", (float) seplos_get_16bit(63) * 0.1f);

  //   65     0x46 0x50      Rated capacity                   18000 * 0.01f = 180.00        Ah
  ESP_LOGD(TAG, "Rated capacity: %.2f %%", (float) seplos_get_16bit(65) * 0.01f);

  //   67     0x00 0x46      Number of cycles                 70
  ESP_LOGD(TAG, "Rated capacity: %.0f %%", (float) seplos_get_16bit(67));

  //   69     0x03 0xE8      State of health                  1000 * 0.1f = 100.0           %
  ESP_LOGD(TAG, "State of health: %.1f %%", (float) seplos_get_16bit(69) * 0.1f);

  //   71     0x14 0x9F      Port voltage                     5279 * 0.01f = 52.79          V
  ESP_LOGD(TAG, "Port voltage: %.2f V", (float) seplos_get_16bit(71) * 0.01f);

  //   73     0x00 0x00      Reserved
  //   75     0x00 0x00      Reserved
  //   77     0x00 0x00      Reserved
  //   79     0x00 0x00      Reserved
}

void SeplosBms::dump_config() {
  ESP_LOGCONFIG(TAG, "SeplosBms:");
  // @TODO
}

float SeplosBms::get_setup_priority() const {
  // After UART bus
  return setup_priority::BUS - 1.0f;
}

void SeplosBms::update() { this->send(); }

void SeplosBms::publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state) {
  if (binary_sensor == nullptr)
    return;

  binary_sensor->publish_state(state);
}

void SeplosBms::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor == nullptr)
    return;

  sensor->publish_state(value);
}

void SeplosBms::publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state) {
  if (text_sensor == nullptr)
    return;

  text_sensor->publish_state(state);
}

}  // namespace seplos_bms
}  // namespace esphome
