#include "seplos_bms.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace seplos_bms {

static const char *const TAG = "seplos_bms";

void SeplosBms::on_seplos_modbus_data(const std::vector<uint8_t> &data) {
  // num_of_cells   frame_size   data_len
  // 8              58          104 (0x68)        65           118 (0x76)   guessed
  // 14             70          128 (0x80)        77           142 (0x8E)
  // 15             72          132 (0x84)        79           146 (0x92)
  // 16             74          136 (0x88)        81           150 (0x96)
  if (data.size() >= 44 && data[8] >= 8 && data[8] <= 16) {
    this->on_telemetry_data_(data);
    return;
  }

  ESP_LOGW(TAG, "Unhandled data received (data_len: 0x%02X): %s", data[5],
           format_hex_pretty(&data.front(), data.size()).c_str());
}

void SeplosBms::on_telemetry_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGI(TAG, "Telemetry frame (%d bytes) received", data.size());
  ESP_LOGVV(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // clang-format off
  //
  // ->
  // 0x20004600109600 01 10 0CD7 0CE9  0CF4 0CD6 0CEF  0CE5  0CE1 0CDC 0CE9 0CF0 0CE8 0CEF 0CEA 0CDA 0CDE 0CD8 06  0BA6 0BA0 0B97 0BA6 0BA5 0BA2  FD5C  14A0 344E 0A 4268  0313  4650  0046 03E8 149F   0000 0000 0000 0000
  //    0 1 2 3 4 5 6 7  8   9   11    13   15   17    19    21   23   25   27   29   31   33   35   37   39   41  42   44   46   48   50   52    54    56   58   60 61    63    65    67   69   71     73   75   77   79
  // 0x25014F42808000 01 0E 0DD2 0DD2  0DD3 0DD3 0DCC  0DCC  0DCD 0DCD 0DCE 0DCF 0DCD 0DCE 0DCE 0DCE           06  0BE2 0BB1 0B9A 0B9A 0B9A 0BA2  0000  1354 395A 00 5B68  0000  5B68  133A 09BE 3F    0409
  //    0 1 2 3 4 5 6 7  8   9   11    13   15   17    19    21   23   25   27   29   31   33   35             37  38   40   42   44   46   48    50    52   54   56 57    59    61    63   65   67    68
  //                                                                                                           37  38   40   42   44   46   48    50    52   54   56 57    59    61    63   65   67    68       razem 70 byte
  //                1 2  3   4   6     8    10   12    14    16   18   20   22   24   26   28   30             32  33   35   37   39   41   43    45    47   49   51 52    54    56    58   60   62    6364
  //
  // clang-format on

  // *Data*
  //
  // Byte   Address Content: Description                      Decoded content               Coeff./Unit
  //   0    0x25             Protocol version      VER        2.5
  //   1    0x01             Device address        ADR
  //   2    0x4F             Device type           CID1       Lithium iron phosphate battery BMS
  //   3    0x42             Function code         CID2       0x00: Normal, 0x01 VER error, 0x02 Chksum error, ...
  //   4    0x80            Data length checksum  LCHKSUM
  //   5    0x80            Data length           LENID      128 / 2 = 64
  //   6      0x00           Data flag
  //   7      0x01           Command group
  ESP_LOGV(TAG, "Command group: %d", data[7]);
  //   8      0x0E           Number of cells                  14
  uint8_t cells = (this->override_cell_count_) ? this->override_cell_count_ : data[8];

  ESP_LOGV(TAG, "Number of cells: %d", cells);
  //   9      0x0D 0xD2      Cell voltage 1                   3538 * 0.001f = 3.538         V
  //   11     0x0D 0xD2      Cell voltage 2                   3538 * 0.001f = 3.538         V
  //   ...    ...            ...
  //   39     0x0D 0xCE      Cell voltage 14                                                V
  float min_cell_voltage = 100.0f;
  float max_cell_voltage = -100.0f;
  float average_cell_voltage = 0.0f;
  uint8_t min_voltage_cell = 0;
  uint8_t max_voltage_cell = 0;
  for (uint8_t i = 0; i < std::min((uint8_t) 16, cells); i++) {
    float cell_voltage = (float) seplos_get_16bit(9 + (i * 2)) * 0.001f;
    average_cell_voltage = average_cell_voltage + cell_voltage;
    if (cell_voltage < min_cell_voltage) {
      min_cell_voltage = cell_voltage;
      min_voltage_cell = i + 1;
    }
    if (cell_voltage > max_cell_voltage) {
      max_cell_voltage = cell_voltage;
      max_voltage_cell = i + 1;
    }
    this->publish_state_(this->cells_[i].cell_voltage_sensor_, cell_voltage);
  }
  average_cell_voltage = average_cell_voltage / cells;

  this->publish_state_(this->min_cell_voltage_sensor_, min_cell_voltage);
  this->publish_state_(this->max_cell_voltage_sensor_, max_cell_voltage);
  this->publish_state_(this->max_voltage_cell_sensor_, (float) max_voltage_cell);
  this->publish_state_(this->min_voltage_cell_sensor_, (float) min_voltage_cell);
  this->publish_state_(this->delta_cell_voltage_sensor_, max_cell_voltage - min_cell_voltage);
  this->publish_state_(this->average_cell_voltage_sensor_, average_cell_voltage);

  uint8_t offset = 9 + (cells * 2);
  //  9+28=37
  //  37  41     0x06           Number of temperatures           6                             V
  uint8_t temperature_sensors = data[offset];
  ESP_LOGV(TAG, "Number of temperature sensors: %d", temperature_sensors);

  // 38   42     0x0B 0xE2      Environment temperature          (3042 - 2731) * 0.1f = 31.1          °C
  // 40   44     0x0B 0xB1      Mosfet temperature               (2993 - 2731) * 0.1f = 26.2          °C
  // 42   46     0x0B 0x9A      Temperature sensor 1             (2970 - 2731) * 0.1f = 23.9          °C
  // 44   48     0x0B 0x9A      Temperature sensor 2             (2970 - 2731) * 0.1f = 23.9          °C
  // 46   50     0x0B 0x9A      Temperature sensor 3             (2970 - 2731) * 0.1f = 23.9          °C
  // 48   52     0x0B 0xA2      Temperature sensor 4             (2978 - 2731) * 0.1f = 24.7          °C
  for (uint8_t i = 0; i < std::min((uint8_t) 6, temperature_sensors); i++) {
    float raw_temperature = (float) seplos_get_16bit(offset + 1 + (i * 2));
    this->publish_state_(this->temperatures_[i].temperature_sensor_, (raw_temperature - 2731.0f) * 0.1f);
  }
  offset = offset + 1 + (temperature_sensors * 2);

  // 50  54     0x00 0x00      Charge/discharge current         signed int?                   A
  float current = (float) ((int16_t) seplos_get_16bit(offset)) * 0.01f;
  this->publish_state_(this->current_sensor_, current);

  // 52  56     0x13 0x54      Total battery voltage            4948 * 0.01f = 49.48          V
  float total_voltage = (float) seplos_get_16bit(offset + 2) * 0.01f;
  this->publish_state_(this->total_voltage_sensor_, total_voltage);

  float power = total_voltage * current;
  this->publish_state_(this->power_sensor_, power);
  this->publish_state_(this->charging_power_sensor_, std::max(0.0f, power));               // 500W vs 0W -> 500W
  this->publish_state_(this->discharging_power_sensor_, std::abs(std::min(0.0f, power)));  // -500W vs 0W -> 500W

  // 54  58     0x39 0x5A      Residual capacity                14682 * 0.01f = 146.82        Ah
  this->publish_state_(this->residual_capacity_sensor_, (float) seplos_get_16bit(offset + 4) * 0.01f);

  // 56  60     0x0A           Custom number                    10
  // 57  61     0x5B 0x68      Battery capacity                 23400 * 0.01f = 234.00        Ah
  this->publish_state_(this->battery_capacity_sensor_, (float) seplos_get_16bit(offset + 7) * 0.01f);

  // 59  63      0x00 0x00                      Number of cycles                0
  this->publish_state_(this->charging_cycles_sensor_, (float) seplos_get_16bit(offset + 9) * 1.0f);

  // 61  65     0x5B 0x68      Rated capacity                   23400 * 0.01f = 234.00        Ah
  this->publish_state_(this->rated_capacity_sensor_, (float) seplos_get_16bit(offset + 11) * 0.01f);

  if (data.size() < offset + 13 + 2) {
    return;
  }

  // 63  67       0x13 0x3A                    Port voltage            4922 * 0.01f = 49.22     V
  this->publish_state_(this->port_voltage_sensor_, (float) seplos_get_16bit(offset + 13) * 0.01f);

  if (data.size() < offset + 15 + 2) {
    return;
  }

  // 65  69     0x09 0xBE      Sampling Voltage                  2494 * 0.001f = 2.494       V
  this->publish_state_(this->sampling_voltage_sensor_, (float) seplos_get_16bit(offset + 15) * 0.001f);

  // 67   71      0x3F        Stage of charge                63  %
  this->publish_state_(this->state_of_charge_sensor_, (float) data[offset + 17] * 1.0f);

  //   68     0x04 0x09     Hardware version  1033
  //  this->publish_state_(this->hardware_version_sensor_, (float) seplos_get_16bit(offset + 18) * 0.001f);
}

void SeplosBms::dump_config() {
  ESP_LOGCONFIG(TAG, "SeplosBms:");
  LOG_SENSOR("", "Minimum Cell Voltage", this->min_cell_voltage_sensor_);
  LOG_SENSOR("", "Maximum Cell Voltage", this->max_cell_voltage_sensor_);
  LOG_SENSOR("", "Minimum Voltage Cell", this->min_voltage_cell_sensor_);
  LOG_SENSOR("", "Maximum Voltage Cell", this->max_voltage_cell_sensor_);
  LOG_SENSOR("", "Delta Cell Voltage", this->delta_cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 1", this->cells_[0].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 2", this->cells_[1].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 3", this->cells_[2].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 4", this->cells_[3].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 5", this->cells_[4].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 6", this->cells_[5].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 7", this->cells_[6].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 8", this->cells_[7].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 9", this->cells_[8].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 10", this->cells_[9].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 11", this->cells_[10].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 12", this->cells_[11].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 13", this->cells_[12].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 14", this->cells_[13].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 15", this->cells_[14].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 16", this->cells_[15].cell_voltage_sensor_);
  LOG_SENSOR("", "Temperature 1", this->temperatures_[0].temperature_sensor_);
  LOG_SENSOR("", "Temperature 2", this->temperatures_[1].temperature_sensor_);
  LOG_SENSOR("", "Temperature 3", this->temperatures_[2].temperature_sensor_);
  LOG_SENSOR("", "Temperature 4", this->temperatures_[3].temperature_sensor_);
  LOG_SENSOR("", "Temperature 5", this->temperatures_[4].temperature_sensor_);
  LOG_SENSOR("", "Temperature 6", this->temperatures_[5].temperature_sensor_);
  LOG_SENSOR("", "Total Voltage", this->total_voltage_sensor_);
  LOG_SENSOR("", "Current", this->current_sensor_);
  LOG_SENSOR("", "Power", this->power_sensor_);
  LOG_SENSOR("", "Charging Power", this->charging_power_sensor_);
  LOG_SENSOR("", "Discharging Power", this->discharging_power_sensor_);
  LOG_SENSOR("", "Charging cycles", this->charging_cycles_sensor_);
  LOG_SENSOR("", "State of charge", this->state_of_charge_sensor_);
  LOG_SENSOR("", "Residual capacity", this->residual_capacity_sensor_);
  LOG_SENSOR("", "Battery capacity", this->battery_capacity_sensor_);
  LOG_SENSOR("", "Rated capacity", this->rated_capacity_sensor_);
  LOG_SENSOR("", "Charging cycles", this->charging_cycles_sensor_);
  LOG_SENSOR("", "Sampling voltage", this->sampling_voltage_sensor_);
  LOG_SENSOR("", "Port Voltage", this->port_voltage_sensor_);
}

float SeplosBms::get_setup_priority() const {
  // After UART bus
  return setup_priority::BUS - 1.0f;
}

void SeplosBms::update() { this->send(0x42, this->pack_); }

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
