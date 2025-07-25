#include "seplos_bms.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome::seplos_bms {

static constexpr char TAG[] = "seplos_bms";

static constexpr uint8_t MAX_NO_RESPONSE_COUNT = 5;

// clang-format off
static constexpr const char *const ALARM_EVENT1_NAMES[] = {
    "Voltage sensor fault",
    "Temperature sensor fault",
    "Current sensor fault",
    "Key switch fault",
    "Cell voltage dropout fault",
    "Charge switch fault",
    "Discharge switch fault",
    "Current limit switch fault"
};

static constexpr const char *const ALARM_EVENT2_NAMES[] = {
    "Cell high voltage alarm",
    "Cell overvoltage protection",
    "Cell low voltage alarm",
    "Cell undervoltage protection",
    "Pack high voltage alarm",
    "Pack overvoltage protection",
    "Pack low voltage alarm",
    "Pack undervoltage protection"
};

static constexpr const char *const ALARM_EVENT3_NAMES[] = {
    "Charge high temp alarm",
    "Charge overtemp protection",
    "Charge low temp alarm",
    "Charge undertemp protection",
    "Discharge high temp alarm",
    "Discharge overtemp protection",
    "Discharge low temp alarm",
    "Discharge undertemp protection"
};

static constexpr const char *const ALARM_EVENT4_NAMES[] = {
    "Env high temp alarm",
    "Env overtemp protection",
    "Env low temp alarm",
    "Env undertemp protection",
    "Power overtemp protection",
    "Power high temp alarm",
    "Cell low temp heating",
    "Reserved"
};

static constexpr const char *const ALARM_EVENT5_NAMES[] = {
    "Charge overcurrent alarm",
    "Charge overcurrent protection",
    "Discharge overcurrent alarm",
    "Discharge overcurrent protection",
    "Transient overcurrent protection",
    "Output short circuit protection",
    "Transient overcurrent lockout",
    "Output short circuit lockout"
};

static constexpr const char *const ALARM_EVENT6_NAMES[] = {
    "Charge high voltage protection",
    "Intermittent recharge waiting",
    "Residual capacity alarm",
    "Residual capacity protection",
    "Cell low voltage charging prohibition",
    "Output reverse polarity protection",
    "Output connection fault",
    "Inside bit"
};

static constexpr const char *const ALARM_EVENT7_NAMES[] = {
    "Inside bit",
    "Inside bit",
    "Inside bit",
    "Inside bit",
    "Automatic charging waiting",
    "Manual charging waiting",
    "Inside bit",
    "Inside bit"
};

static constexpr const char *const ALARM_EVENT8_NAMES[] = {
    "EEP storage fault",
    "RTC error",
    "Voltage calibration not performed",
    "Current calibration not performed",
    "Zero calibration not performed",
    "Inside bit",
    "Inside bit",
    "Inside bit"
};
// clang-format on

void SeplosBms::on_seplos_modbus_data(const std::vector<uint8_t> &data) {
  this->reset_online_status_tracker_();

  // Alarm frames (CID2=0x44 response) are structurally smaller than telemetry frames.
  // For M=8..16 cells, N=6 temps: alarm max=55 bytes, telemetry min=61 bytes.
  // num_of_cells   telemetry_frame_size   alarm_frame_size
  // 8              ~65                    43-45
  // 14             77                     51
  // 15             79                     52-54
  // 16             81                     55
  if (data.size() >= 9 && data.size() < 60 && data[8] >= 8 && data[8] <= 16) {
    this->on_alarm_data_(data);
    return;
  }

  if (data.size() >= 44 && data[8] >= 8 && data[8] <= 16) {
    this->on_telemetry_data_(data);
    if (this->parent_ != nullptr)
      this->send(0x44, this->pack_);
    return;
  }

  ESP_LOGW(TAG, "Unhandled data received (data_len: 0x%02X): %s", data[5],
           format_hex_pretty(&data.front(), data.size()).c_str());  // NOLINT
}

void SeplosBms::on_telemetry_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGI(TAG, "Telemetry frame (%zu bytes) received", data.size());
  ESP_LOGVV(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());  // NOLINT

  // ->
  // 0x2000460010960001100CD70CE90CF40CD60CEF0CE50CE10CDC0CE90CF00CE80CEF0CEA0CDA0CDE0CD8060BA60BA00B970BA60BA50BA2FD5C14A0344E0A426803134650004603E8149F0000000000000000
  // 0x26004600307600011000000000000000000000000000000000000000000000000000000000000000000608530853085308530BAC0B9000000000002D0213880001E6B8
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
  ESP_LOGV(TAG, "Command group: %d", data[7]);
  //   8      0x10           Number of cells                  16
  uint8_t cells = (this->override_cell_count_) ? this->override_cell_count_ : data[8];

  ESP_LOGV(TAG, "Number of cells: %d", cells);
  //   9      0x0C 0xD7      Cell voltage 1                   3287 * 0.001f = 3.287         V
  //   11     0x0C 0xE9      Cell voltage 2                   3305 * 0.001f = 3.305         V
  //   ...    ...            ...
  //   39     0x0C 0xD8      Cell voltage 16                                                V
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

  //   41     0x06           Number of temperatures           6                             V
  uint8_t temperature_sensors = data[offset];
  ESP_LOGV(TAG, "Number of temperature sensors: %d", temperature_sensors);

  //   42     0x0B 0xA6      Temperature sensor 1             (2982 - 2731) * 0.1f = 25.1          °C
  //   44     0x0B 0xA0      Temperature sensor 2             (2976 - 2731) * 0.1f = 24.5          °C
  //   46     0x0B 0x97      Temperature sensor 3             (2967 - 2731) * 0.1f = 23.6          °C
  //   48     0x0B 0xA6      Temperature sensor 4             (2982 - 2731) * 0.1f = 25.1          °C
  //   50     0x0B 0xA5      Environment temperature          (2981 - 2731) * 0.1f = 25.0          °C
  //   52     0x0B 0xA2      Mosfet temperature               (2978 - 2731) * 0.1f = 24.7          °C
  for (uint8_t i = 0; i < std::min((uint8_t) 6, temperature_sensors); i++) {
    float raw_temperature = (float) seplos_get_16bit(offset + 1 + (i * 2));
    this->publish_state_(this->temperatures_[i].temperature_sensor_, (raw_temperature - 2731.0f) * 0.1f);
  }
  offset = offset + 1 + (temperature_sensors * 2);

  //   54     0xFD 0x5C      Charge/discharge current         signed int?                   A
  float current = (float) ((int16_t) seplos_get_16bit(offset)) * 0.01f;
  this->publish_state_(this->current_sensor_, current);

  //   56     0x14 0xA0      Total battery voltage            5280 * 0.01f = 52.80          V
  float total_voltage = (float) seplos_get_16bit(offset + 2) * 0.01f;
  this->publish_state_(this->total_voltage_sensor_, total_voltage);

  float power = total_voltage * current;
  this->publish_state_(this->power_sensor_, power);
  this->publish_state_(this->charging_power_sensor_, std::max(0.0f, power));               // 500W vs 0W -> 500W
  this->publish_state_(this->discharging_power_sensor_, std::abs(std::min(0.0f, power)));  // -500W vs 0W -> 500W

  //   58     0x34 0x4E      Residual capacity                13390 * 0.01f = 133.90        Ah
  this->publish_state_(this->residual_capacity_sensor_, (float) seplos_get_16bit(offset + 4) * 0.01f);

  //   60     0x0A           Custom number                    10
  //   61     0x42 0x68      Battery capacity                 17000 * 0.01f = 170.00        Ah
  this->publish_state_(this->battery_capacity_sensor_, (float) seplos_get_16bit(offset + 7) * 0.01f);

  //   63     0x03 0x13      Stage of charge                  787 * 0.1f = 78.7             %
  this->publish_state_(this->state_of_charge_sensor_, (float) seplos_get_16bit(offset + 9) * 0.1f);

  //   65     0x46 0x50      Rated capacity                   18000 * 0.01f = 180.00        Ah
  this->publish_state_(this->rated_capacity_sensor_, (float) seplos_get_16bit(offset + 11) * 0.01f);

  if (data.size() < offset + 13 + 2) {
    return;
  }

  //   67     0x00 0x46      Number of cycles                 70
  this->publish_state_(this->charging_cycles_sensor_, (float) seplos_get_16bit(offset + 13));

  if (data.size() < offset + 15 + 2) {
    return;
  }

  //   69     0x03 0xE8      State of health                  1000 * 0.1f = 100.0           %
  this->publish_state_(this->state_of_health_sensor_, (float) seplos_get_16bit(offset + 15) * 0.1f);

  if (data.size() < offset + 17 + 2) {
    return;
  }

  //   71     0x14 0x9F      Port voltage                     5279 * 0.01f = 52.79          V
  this->publish_state_(this->port_voltage_sensor_, (float) seplos_get_16bit(offset + 17) * 0.01f);

  //   73     0x00 0x00      Reserved
  //   75     0x00 0x00      Reserved
  //   77     0x00 0x00      Reserved
  //   79     0x00 0x00      Reserved
}

void SeplosBms::on_alarm_data_(const std::vector<uint8_t> &data) {
  ESP_LOGI(TAG, "Alarm frame (%zu bytes) received", data.size());
  ESP_LOGVV(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());  // NOLINT

  // Frame layout (Table 11 of SEPLOS BMS Communication Protocol V2.0):
  //   data[6]       = DATA FLAG
  //   data[7]       = COMMAND GROUP (pack address)
  //   data[8]       = M  (number of cell alarm bytes, typically 15 or 16)
  //   data[9..8+M]  = M cell alarm bytes (0x00=normal, 0x01=lower, 0x02=upper, 0xF0=other)
  //   data[9+M]     = N  (number of temperature alarm bytes, typically 6)
  //   data[10+M..9+M+N] = N temperature alarm bytes
  //   data[10+M+N]  = charge/discharge current alarm
  //   data[11+M+N]  = total battery voltage alarm
  //   data[12+M+N]  = P  (number of custom bit alarms = 20)
  //   data[13+M+N]  = alarm event 1  (HW faults)
  //   data[14+M+N]  = alarm event 2  (voltage)
  //   data[15+M+N]  = alarm event 3  (cell temperature)
  //   data[16+M+N]  = alarm event 4  (environment temperature)
  //   data[17+M+N]  = alarm event 5  (current protection)
  //   data[18+M+N]  = alarm event 6  (charge/output protection)
  //   data[19+M+N]  = on-off state   (switch states)
  //   data[20+M+N]  = equilibrium state 1 (cells 1-8)
  //   data[21+M+N]  = equilibrium state 2 (cells 9-16)
  //   data[22+M+N]  = system state
  //   data[23+M+N]  = disconnection state 1 (cells 1-8)
  //   data[24+M+N]  = disconnection state 2 (cells 9-16)
  //   data[25+M+N]  = alarm event 7  (charging wait states)
  //   data[26+M+N]  = alarm event 8  (calibration faults)
  //   data[27+M+N..32+M+N] = reservation (6 bytes)

  if (data.size() < 9) {
    ESP_LOGW(TAG, "Alarm frame too short");
    return;
  }

  uint8_t cell_count = data[8];
  if (cell_count < 8 || cell_count > 16) {
    ESP_LOGW(TAG, "Alarm frame: invalid cell count %d", cell_count);
    return;
  }

  const size_t temp_count_offset = 9 + cell_count;
  if (data.size() <= temp_count_offset) {
    ESP_LOGW(TAG, "Alarm frame too short for temperature count");
    return;
  }

  uint8_t temp_count = data[temp_count_offset];
  if (temp_count > 8) {
    ESP_LOGW(TAG, "Alarm frame: invalid temperature count %d", temp_count);
    return;
  }

  // alarm_events_offset = start of alarm event 1, must have 14 bytes for events 1-8 + states
  const size_t alarm_events_offset = 9 + cell_count + 1 + temp_count + 3;
  if (data.size() < alarm_events_offset + 14) {
    ESP_LOGW(TAG, "Alarm frame too short for alarm events (size=%d, need=%d)", (int) data.size(),
             (int) (alarm_events_offset + 14));
    return;
  }

  uint8_t alarm_event1 = data[alarm_events_offset + 0];   // HW faults
  uint8_t alarm_event2 = data[alarm_events_offset + 1];   // voltage alarms
  uint8_t alarm_event3 = data[alarm_events_offset + 2];   // cell temperature
  uint8_t alarm_event4 = data[alarm_events_offset + 3];   // environment temperature
  uint8_t alarm_event5 = data[alarm_events_offset + 4];   // current protection
  uint8_t alarm_event6 = data[alarm_events_offset + 5];   // charge/output protection
  uint8_t on_off_state = data[alarm_events_offset + 6];   // switch states
  uint8_t eq_state1 = data[alarm_events_offset + 7];      // balancing cells 1-8
  uint8_t eq_state2 = data[alarm_events_offset + 8];      // balancing cells 9-16
  uint8_t system_state = data[alarm_events_offset + 9];   // system operational state
  uint8_t dis_state1 = data[alarm_events_offset + 10];    // disconnection cells 1-8
  uint8_t dis_state2 = data[alarm_events_offset + 11];    // disconnection cells 9-16
  uint8_t alarm_event7 = data[alarm_events_offset + 12];  // charging wait states
  uint8_t alarm_event8 = data[alarm_events_offset + 13];  // calibration faults

  // Voltage alarms (alarm_event2, Table 13)
  ESP_LOGD(TAG, "Alarm event 2 (voltage): 0x%02X", alarm_event2);
  ESP_LOGD(TAG, "  Bit0 Monomer high voltage alarm: %s", ONOFF(alarm_event2 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Monomer overvoltage protection: %s", ONOFF(alarm_event2 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Monomer low voltage alarm: %s", ONOFF(alarm_event2 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Monomer under voltage protection: %s", ONOFF(alarm_event2 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Total voltage high alarm: %s", ONOFF(alarm_event2 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Total voltage overvoltage protection: %s", ONOFF(alarm_event2 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Total voltage low alarm: %s", ONOFF(alarm_event2 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Total voltage undervoltage protection: %s", ONOFF(alarm_event2 & 0x80));
  this->publish_state_(this->voltage_protection_binary_sensor_, (alarm_event2 != 0));

  // Temperature alarms (alarm_event3, Table 13)
  ESP_LOGD(TAG, "Alarm event 3 (temperature): 0x%02X", alarm_event3);
  ESP_LOGD(TAG, "  Bit0 Charge high temperature alarm: %s", ONOFF(alarm_event3 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charge over temperature protection: %s", ONOFF(alarm_event3 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Charge low temperature alarm: %s", ONOFF(alarm_event3 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Charge under temperature protection: %s", ONOFF(alarm_event3 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Discharge high temperature alarm: %s", ONOFF(alarm_event3 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Discharge over temperature protection: %s", ONOFF(alarm_event3 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Discharge low temperature alarm: %s", ONOFF(alarm_event3 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Discharge under temperature protection: %s", ONOFF(alarm_event3 & 0x80));
  this->publish_state_(this->temperature_protection_binary_sensor_, (alarm_event3 != 0));

  // Current protection (alarm_event5, Table 13)
  ESP_LOGD(TAG, "Alarm event 5 (current): 0x%02X", alarm_event5);
  ESP_LOGD(TAG, "  Bit0 Charging overcurrent alarm: %s", ONOFF(alarm_event5 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charging overcurrent protection: %s", ONOFF(alarm_event5 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Discharge overcurrent alarm: %s", ONOFF(alarm_event5 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Discharge overcurrent protection: %s", ONOFF(alarm_event5 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Transient overcurrent protection: %s", ONOFF(alarm_event5 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Output short circuit protection: %s", ONOFF(alarm_event5 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Transient overcurrent lockout: %s", ONOFF(alarm_event5 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Output short circuit lockout: %s", ONOFF(alarm_event5 & 0x80));
  this->publish_state_(this->current_protection_binary_sensor_, (alarm_event5 != 0));

  // Output/SOC protection (alarm_event6, Table 13)
  ESP_LOGD(TAG, "Alarm event 6 (output): 0x%02X", alarm_event6);
  ESP_LOGD(TAG, "  Bit0 Charging high voltage protection: %s", ONOFF(alarm_event6 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Intermittent recharge waiting: %s", ONOFF(alarm_event6 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Residual capacity alarm: %s", ONOFF(alarm_event6 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Residual capacity protection: %s", ONOFF(alarm_event6 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Cell low voltage charging prohibition: %s", ONOFF(alarm_event6 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Output reverse polarity protection: %s", ONOFF(alarm_event6 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Output connection fault: %s", ONOFF(alarm_event6 & 0x40));
  this->publish_state_(this->soc_protection_binary_sensor_, (alarm_event6 != 0));

  // Switch states (on-off state, Table 13)
  ESP_LOGD(TAG, "On-off state: 0x%02X", on_off_state);
  ESP_LOGD(TAG, "  Bit0 Discharge switch: %s", ONOFF(on_off_state & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charge switch: %s", ONOFF(on_off_state & 0x02));
  ESP_LOGD(TAG, "  Bit2 Current limit switch: %s", ONOFF(on_off_state & 0x04));
  ESP_LOGD(TAG, "  Bit3 Heating switch: %s", ONOFF(on_off_state & 0x08));
  this->publish_state_(this->discharging_binary_sensor_, (on_off_state & 0x01) != 0);
  this->publish_state_(this->charging_binary_sensor_, (on_off_state & 0x02) != 0);

  // Alarm event bitmask sensors
  this->publish_state_(this->alarm_event1_bitmask_sensor_, (float) alarm_event1);
  this->publish_state_(this->alarm_event2_bitmask_sensor_, (float) alarm_event2);
  this->publish_state_(this->alarm_event3_bitmask_sensor_, (float) alarm_event3);
  this->publish_state_(this->alarm_event4_bitmask_sensor_, (float) alarm_event4);
  this->publish_state_(this->alarm_event5_bitmask_sensor_, (float) alarm_event5);
  this->publish_state_(this->alarm_event6_bitmask_sensor_, (float) alarm_event6);
  this->publish_state_(this->alarm_event7_bitmask_sensor_, (float) alarm_event7);
  this->publish_state_(this->alarm_event8_bitmask_sensor_, (float) alarm_event8);

  // Balancing states
  uint16_t balancing_combined = uint16_t(eq_state1) | (uint16_t(eq_state2) << 8);
  ESP_LOGD(TAG, "Balancing: eq1=0x%02X eq2=0x%02X combined=0x%04X", eq_state1, eq_state2, balancing_combined);
  this->publish_state_(this->balancing_bitmask_sensor_, (float) balancing_combined);
  this->publish_state_(this->balancing_binary_sensor_, balancing_combined != 0);

  // System state
  ESP_LOGD(TAG, "System state: 0x%02X", system_state);
  ESP_LOGD(TAG, "  Bit0 Discharge: %s", ONOFF(system_state & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charge: %s", ONOFF(system_state & 0x02));
  ESP_LOGD(TAG, "  Bit2 Floating charge: %s", ONOFF(system_state & 0x04));
  ESP_LOGD(TAG, "  Bit4 Standby: %s", ONOFF(system_state & 0x10));
  ESP_LOGD(TAG, "  Bit5 Shutdown: %s", ONOFF(system_state & 0x20));
  this->publish_state_(this->system_status_bitmask_sensor_, (float) system_state);

  // Disconnection states
  uint16_t disconnection_combined = uint16_t(dis_state1) | (uint16_t(dis_state2) << 8);
  ESP_LOGD(TAG, "Disconnection: dis1=0x%02X dis2=0x%02X combined=0x%04X", dis_state1, dis_state2,
           disconnection_combined);
  this->publish_state_(this->disconnection_bitmask_sensor_, (float) disconnection_combined);

  // Consolidated alarm text
  std::string all_alarms = this->decode_all_alarm_events_(alarm_event1, alarm_event2, alarm_event3, alarm_event4,
                                                          alarm_event5, alarm_event6, alarm_event7, alarm_event8);
  this->publish_state_(this->alarms_text_sensor_, all_alarms);
}

void SeplosBms::dump_config() {
  ESP_LOGCONFIG(TAG, "SeplosBms:");
  LOG_BINARY_SENSOR("", "Charging", this->charging_binary_sensor_);
  LOG_BINARY_SENSOR("", "Discharging", this->discharging_binary_sensor_);
  LOG_BINARY_SENSOR("", "Voltage Protection", this->voltage_protection_binary_sensor_);
  LOG_BINARY_SENSOR("", "Temperature Protection", this->temperature_protection_binary_sensor_);
  LOG_BINARY_SENSOR("", "Current Protection", this->current_protection_binary_sensor_);
  LOG_BINARY_SENSOR("", "SOC Protection", this->soc_protection_binary_sensor_);
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
  LOG_SENSOR("", "Average Cell Voltage", this->average_cell_voltage_sensor_);
  LOG_SENSOR("", "State of health", this->state_of_health_sensor_);
  LOG_SENSOR("", "Port Voltage", this->port_voltage_sensor_);

  LOG_SENSOR("", "Alarm Event 1 Bitmask", this->alarm_event1_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 2 Bitmask", this->alarm_event2_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 3 Bitmask", this->alarm_event3_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 4 Bitmask", this->alarm_event4_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 5 Bitmask", this->alarm_event5_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 6 Bitmask", this->alarm_event6_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 7 Bitmask", this->alarm_event7_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 8 Bitmask", this->alarm_event8_bitmask_sensor_);
  LOG_SENSOR("", "Balancing Bitmask", this->balancing_bitmask_sensor_);
  LOG_SENSOR("", "Disconnection Bitmask", this->disconnection_bitmask_sensor_);
  LOG_SENSOR("", "System Status Bitmask", this->system_status_bitmask_sensor_);

  LOG_BINARY_SENSOR("", "Balancing", this->balancing_binary_sensor_);

  LOG_TEXT_SENSOR("", "Alarms", this->alarms_text_sensor_);
}

float SeplosBms::get_setup_priority() const {
  // After UART bus
  return setup_priority::BUS - 1.0f;
}

void SeplosBms::update() {
  this->track_online_status_();
  this->send(0x42, this->pack_);
}

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

void SeplosBms::track_online_status_() {
  if (this->no_response_count_ < MAX_NO_RESPONSE_COUNT) {
    this->no_response_count_++;
  }
  if (this->no_response_count_ == MAX_NO_RESPONSE_COUNT) {
    this->publish_device_unavailable_();
    this->no_response_count_++;
  }
}

void SeplosBms::reset_online_status_tracker_() {
  this->no_response_count_ = 0;
  this->publish_state_(this->online_status_binary_sensor_, true);
}

void SeplosBms::publish_device_unavailable_() {
  this->publish_state_(this->online_status_binary_sensor_, false);
  this->publish_state_(this->charging_binary_sensor_, false);
  this->publish_state_(this->discharging_binary_sensor_, false);
  this->publish_state_(this->balancing_binary_sensor_, false);
  this->publish_state_(this->voltage_protection_binary_sensor_, false);
  this->publish_state_(this->temperature_protection_binary_sensor_, false);
  this->publish_state_(this->current_protection_binary_sensor_, false);
  this->publish_state_(this->soc_protection_binary_sensor_, false);
  this->publish_state_(this->errors_text_sensor_, "Offline");

  this->publish_state_(this->min_cell_voltage_sensor_, NAN);
  this->publish_state_(this->max_cell_voltage_sensor_, NAN);
  this->publish_state_(this->min_voltage_cell_sensor_, NAN);
  this->publish_state_(this->max_voltage_cell_sensor_, NAN);
  this->publish_state_(this->delta_cell_voltage_sensor_, NAN);
  this->publish_state_(this->average_cell_voltage_sensor_, NAN);
  this->publish_state_(this->total_voltage_sensor_, NAN);
  this->publish_state_(this->current_sensor_, NAN);
  this->publish_state_(this->power_sensor_, NAN);
  this->publish_state_(this->charging_power_sensor_, NAN);
  this->publish_state_(this->discharging_power_sensor_, NAN);
  this->publish_state_(this->state_of_charge_sensor_, NAN);
  this->publish_state_(this->residual_capacity_sensor_, NAN);
  this->publish_state_(this->battery_capacity_sensor_, NAN);
  this->publish_state_(this->rated_capacity_sensor_, NAN);
  this->publish_state_(this->charging_cycles_sensor_, NAN);
  this->publish_state_(this->state_of_health_sensor_, NAN);
  this->publish_state_(this->port_voltage_sensor_, NAN);

  // Alarm event bitmask sensors
  this->publish_state_(this->alarm_event1_bitmask_sensor_, NAN);
  this->publish_state_(this->alarm_event2_bitmask_sensor_, NAN);
  this->publish_state_(this->alarm_event3_bitmask_sensor_, NAN);
  this->publish_state_(this->alarm_event4_bitmask_sensor_, NAN);
  this->publish_state_(this->alarm_event5_bitmask_sensor_, NAN);
  this->publish_state_(this->alarm_event6_bitmask_sensor_, NAN);
  this->publish_state_(this->alarm_event7_bitmask_sensor_, NAN);
  this->publish_state_(this->alarm_event8_bitmask_sensor_, NAN);

  // Balancing and disconnection sensors
  this->publish_state_(this->balancing_bitmask_sensor_, NAN);
  this->publish_state_(this->disconnection_bitmask_sensor_, NAN);
  this->publish_state_(this->system_status_bitmask_sensor_, NAN);

  // Text sensors
  this->publish_state_(this->alarms_text_sensor_, "Offline");

  for (auto &temperature : this->temperatures_) {
    this->publish_state_(temperature.temperature_sensor_, NAN);
  }

  for (auto &cell : this->cells_) {
    this->publish_state_(cell.cell_voltage_sensor_, NAN);
  }
}

std::string SeplosBms::bitmask_to_string_(const char *const messages[], const uint8_t &messages_size,
                                          const uint8_t &mask) {
  std::string alarm_info;
  if (mask) {
    for (uint8_t i = 0; i < messages_size; i++) {
      if (mask & (1 << i)) {
        if (!alarm_info.empty()) {
          alarm_info.append("; ");
        }
        alarm_info.append(messages[i]);
      }
    }
  }
  return alarm_info;
}

std::string SeplosBms::decode_all_alarm_events_(uint8_t alarm_event1, uint8_t alarm_event2, uint8_t alarm_event3,
                                                uint8_t alarm_event4, uint8_t alarm_event5, uint8_t alarm_event6,
                                                uint8_t alarm_event7, uint8_t alarm_event8) {
  std::string alarm_text;

  std::string alarm_event1_text = this->bitmask_to_string_(ALARM_EVENT1_NAMES, 8, alarm_event1);
  if (!alarm_event1_text.empty()) {
    if (!alarm_text.empty())
      alarm_text.append("; ");
    alarm_text.append("HW: ").append(alarm_event1_text);
  }

  std::string alarm_event2_text = this->bitmask_to_string_(ALARM_EVENT2_NAMES, 8, alarm_event2);
  if (!alarm_event2_text.empty()) {
    if (!alarm_text.empty())
      alarm_text.append("; ");
    alarm_text.append("VOLT: ").append(alarm_event2_text);
  }

  std::string alarm_event3_text = this->bitmask_to_string_(ALARM_EVENT3_NAMES, 8, alarm_event3);
  if (!alarm_event3_text.empty()) {
    if (!alarm_text.empty())
      alarm_text.append("; ");
    alarm_text.append("TEMP: ").append(alarm_event3_text);
  }

  std::string alarm_event4_text = this->bitmask_to_string_(ALARM_EVENT4_NAMES, 8, alarm_event4);
  if (!alarm_event4_text.empty()) {
    if (!alarm_text.empty())
      alarm_text.append("; ");
    alarm_text.append("ENV: ").append(alarm_event4_text);
  }

  std::string alarm_event5_text = this->bitmask_to_string_(ALARM_EVENT5_NAMES, 8, alarm_event5);
  if (!alarm_event5_text.empty()) {
    if (!alarm_text.empty())
      alarm_text.append("; ");
    alarm_text.append("CURR: ").append(alarm_event5_text);
  }

  std::string alarm_event6_text = this->bitmask_to_string_(ALARM_EVENT6_NAMES, 8, alarm_event6);
  if (!alarm_event6_text.empty()) {
    if (!alarm_text.empty())
      alarm_text.append("; ");
    alarm_text.append("CHG: ").append(alarm_event6_text);
  }

  std::string alarm_event7_text = this->bitmask_to_string_(ALARM_EVENT7_NAMES, 8, alarm_event7);
  if (!alarm_event7_text.empty()) {
    if (!alarm_text.empty())
      alarm_text.append("; ");
    alarm_text.append("SYS: ").append(alarm_event7_text);
  }

  std::string alarm_event8_text = this->bitmask_to_string_(ALARM_EVENT8_NAMES, 8, alarm_event8);
  if (!alarm_event8_text.empty()) {
    if (!alarm_text.empty())
      alarm_text.append("; ");
    alarm_text.append("CAL: ").append(alarm_event8_text);
  }

  return alarm_text.empty() ? "No alarms" : alarm_text;
}

}  // namespace esphome::seplos_bms
