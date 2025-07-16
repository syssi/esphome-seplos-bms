#include "seplos_bms_v3_ble.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "../seplos_bms_v3_ble_pack/seplos_bms_v3_ble_pack.h"
#include <cmath>

namespace esphome {
namespace seplos_bms_v3_ble {

static const char *const TAG = "seplos_bms_v3_ble";

static const uint16_t SEPLOS_BMS_V3_SERVICE_UUID = 0xFFF0;
static const uint16_t SEPLOS_BMS_V3_NOTIFY_CHARACTERISTIC_UUID = 0xFFF1;
static const uint16_t SEPLOS_BMS_V3_CONTROL_CHARACTERISTIC_UUID = 0xFFF2;

static const uint16_t MAX_RESPONSE_SIZE = 300;

static const uint8_t SEPLOS_V3_CMD_READ_04 = 0x04;
static const uint8_t SEPLOS_V3_CMD_READ_01 = 0x01;

static const uint16_t SEPLOS_V3_REG_EIA_START = 0x2000;
static const uint16_t SEPLOS_V3_REG_EIB_START = 0x2100;
static const uint16_t SEPLOS_V3_REG_EIC_START = 0x2200;
static const uint16_t SEPLOS_V3_REG_PIA_START = 0x1000;
static const uint16_t SEPLOS_V3_REG_PIB_START = 0x1100;
static const uint16_t SEPLOS_V3_REG_PIC_START = 0x1240;
static const uint16_t SEPLOS_V3_REG_VIA_START = 0x1700;

static const uint16_t SEPLOS_V3_EIA_LENGTH = 0x11;
static const uint16_t SEPLOS_V3_EIB_LENGTH = 0x1A;
static const uint16_t SEPLOS_V3_EIC_LENGTH = 0x05;
static const uint16_t SEPLOS_V3_PIA_LENGTH = 0x11;
static const uint16_t SEPLOS_V3_PIB_LENGTH = 0x1A;
static const uint16_t SEPLOS_V3_PIC_LENGTH = 0x09;
static const uint16_t SEPLOS_V3_VIA_LENGTH = 0x24;

static const SeplosV3Command SEPLOS_V3_SYSTEM_COMMANDS[] = {
    {0x00, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_EIA_START, SEPLOS_V3_EIA_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_EIB_START, SEPLOS_V3_EIB_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_01, SEPLOS_V3_REG_EIC_START, SEPLOS_V3_EIC_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_VIA_START, SEPLOS_V3_VIA_LENGTH},
};

static const SeplosV3Command SEPLOS_V3_PACK_COMMANDS[] = {
    {0x00, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_PIA_START, SEPLOS_V3_PIA_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_PIB_START, SEPLOS_V3_PIB_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_01, SEPLOS_V3_REG_PIC_START, SEPLOS_V3_PIC_LENGTH},
};

void SeplosBmsV3Ble::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                         esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      this->node_state = espbt::ClientState::IDLE;
      this->publish_state_(this->online_status_binary_sensor_, false);
      this->frame_buffer_.clear();
      this->next_command_ = 0;
      this->pack_count_ = 0;
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      auto *char_notify =
          this->parent_->get_characteristic(SEPLOS_BMS_V3_SERVICE_UUID, SEPLOS_BMS_V3_NOTIFY_CHARACTERISTIC_UUID);
      if (char_notify == nullptr) {
        ESP_LOGE(TAG, "[%s] No notify service found at device, not a Seplos BMS V3..?",
                 this->parent_->address_str().c_str());
        break;
      }
      this->char_notify_handle_ = char_notify->handle;

      auto status = esp_ble_gattc_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(),
                                                      char_notify->handle);
      if (status) {
        ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status);
      }

      auto *char_command =
          this->parent_->get_characteristic(SEPLOS_BMS_V3_SERVICE_UUID, SEPLOS_BMS_V3_CONTROL_CHARACTERISTIC_UUID);
      if (char_command == nullptr) {
        ESP_LOGE(TAG, "[%s] No control service found at device, not a Seplos BMS V3..?",
                 this->parent_->address_str().c_str());
        break;
      }
      this->char_command_handle_ = char_command->handle;
      break;
    }
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      this->node_state = espbt::ClientState::ESTABLISHED;
      this->publish_state_(this->online_status_binary_sensor_, true);

      // Build dynamic command queue and send first command
      this->build_dynamic_command_queue_();
      if (!this->dynamic_command_queue_.empty()) {
        this->send_command_(this->dynamic_command_queue_[0].function,
                            this->build_modbus_payload_(this->dynamic_command_queue_[0]));
      }
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      ESP_LOGVV(TAG, "Notification received: %s",
                format_hex_pretty(param->notify.value, param->notify.value_len).c_str());
      this->assemble(param->notify.value, param->notify.value_len);
      break;
    }
    default:
      break;
  }
}

void SeplosBmsV3Ble::dump_config() {
  ESP_LOGCONFIG(TAG, "Seplos BMS V3 BLE");
  ESP_LOGCONFIG(TAG, "  Update interval: %dms", this->get_update_interval());
}

void SeplosBmsV3Ble::update() {
  if (this->node_state != espbt::ClientState::ESTABLISHED) {
    ESP_LOGW(TAG, "[%s] Not connected", this->parent_->address_str().c_str());
    return;
  }

  if (this->next_command_ != 0) {
    ESP_LOGW(TAG, "Command queue (%d of %d) was not completely processed", this->next_command_,
             this->dynamic_command_queue_.size());
  }

  this->next_command_ = 0;
  this->build_dynamic_command_queue_();
  if (!this->dynamic_command_queue_.empty()) {
    this->send_command_(this->dynamic_command_queue_[this->next_command_].function,
                        this->build_modbus_payload_(this->dynamic_command_queue_[this->next_command_]));
    this->next_command_++;
  }
}

void SeplosBmsV3Ble::assemble(const uint8_t *data, uint16_t length) {
  if (this->frame_buffer_.size() > MAX_RESPONSE_SIZE) {
    ESP_LOGW(TAG, "Frame dropped because of invalid length");
    this->frame_buffer_.clear();
  }

  // Add incoming data to buffer
  this->frame_buffer_.insert(this->frame_buffer_.end(), data, data + length);

  // Check if we have a complete ModBus frame
  if (this->frame_buffer_.size() >= 5) {
    uint8_t data_length = this->frame_buffer_[2];

    uint16_t expected_length = 3 + data_length + 2;  // header + data + CRC

    if (this->frame_buffer_.size() >= expected_length) {
      // Verify CRC
      uint16_t frame_crc = this->frame_buffer_[expected_length - 2] | (this->frame_buffer_[expected_length - 1] << 8);
      uint16_t computed_crc = this->crc16_(this->frame_buffer_.data(), expected_length - 2);

      if (computed_crc == frame_crc) {
        std::vector<uint8_t> complete_frame(this->frame_buffer_.begin(), this->frame_buffer_.begin() + expected_length);
        this->decode(complete_frame);

        // Send next command if available
        if (this->next_command_ < this->dynamic_command_queue_.size()) {
          this->send_command_(this->dynamic_command_queue_[this->next_command_].function,
                              this->build_modbus_payload_(this->dynamic_command_queue_[this->next_command_]));
          this->next_command_++;
        }
      } else {
        ESP_LOGW(TAG, "CRC check failed! 0x%04X != 0x%04X", computed_crc, frame_crc);
      }

      this->frame_buffer_.clear();
    }
  }
}

void SeplosBmsV3Ble::decode(const std::vector<uint8_t> &data) {
  uint8_t device = data[0];
  uint8_t function = data[1];
  uint8_t data_length = data[2];

  ESP_LOGD(TAG, "Decoding frame: device=0x%02X, function=0x%02X, length=%d", device, function, data_length);

  if (function & 0x80) {
    ESP_LOGW(TAG, "Error response from device 0x%02X, error code: 0x%02X", device, data[2]);
    return;
  }

  std::vector<uint8_t> payload(data.begin() + 3, data.end() - 2);

  if (device == 0x00) {
    if (data_length == SEPLOS_V3_EIA_LENGTH * 2) {
      this->decode_eia_data_(payload);
    } else if (data_length == SEPLOS_V3_EIB_LENGTH * 2) {
      this->decode_eib_data_(payload);
    } else if (data_length == SEPLOS_V3_EIC_LENGTH * 2) {
      this->decode_eic_data_(payload);
    } else if (data_length == SEPLOS_V3_VIA_LENGTH * 2) {
      this->decode_via_data_(payload);
    }
  } else if (device >= 1 && device <= 16) {
    // First try the new sub-platform architecture
    bool handled_by_sub_platform = false;

    for (auto *pack_sensor : this->pack_sensors_) {
      if (pack_sensor->get_address() == device) {
        handled_by_sub_platform = true;
        if (data_length == SEPLOS_V3_PIA_LENGTH * 2) {
          this->update_pack_pia_data(device, payload);
        } else if (data_length == SEPLOS_V3_PIB_LENGTH * 2) {
          this->update_pack_pib_data(device, payload);
        } else if (data_length == SEPLOS_V3_PIC_LENGTH * 2) {
          this->update_pack_pic_data(device, payload);
        }
        break;
      }
    }

    // If no sub-platform pack sensor found, log warning
    if (!handled_by_sub_platform) {
      ESP_LOGW(TAG, "No pack sensor found for address: 0x%02X", device);
    }
  }
}

void SeplosBmsV3Ble::decode_eia_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  auto seplos_get_32bit = [&](size_t i) -> uint32_t {
    // Read bytes in swapped order: word-swapped big endian
    return (uint32_t(data[i + 2]) << 24) | (uint32_t(data[i + 3]) << 16) | (uint32_t(data[i + 0]) << 8) |
           (uint32_t(data[i + 1]) << 0);
  };

  ESP_LOGD(TAG, "Decoding EIA data (%d bytes)", data.size());

  // Voltage
  float voltage = seplos_get_32bit(0) * 0.01f;
  this->publish_state_(this->total_voltage_sensor_, voltage);

  // Current
  float current = (int32_t) seplos_get_32bit(4) * 0.1f;
  this->publish_state_(this->current_sensor_, current);

  // Power calculation
  float power = voltage * current;
  this->publish_state_(this->power_sensor_, power);

  if (current > 0) {
    this->publish_state_(this->charging_power_sensor_, power);
    this->publish_state_(this->discharging_power_sensor_, 0.0f);
    this->publish_state_(this->charging_binary_sensor_, true);
    this->publish_state_(this->discharging_binary_sensor_, false);
  } else {
    this->publish_state_(this->charging_power_sensor_, 0.0f);
    this->publish_state_(this->discharging_power_sensor_, -power);
    this->publish_state_(this->charging_binary_sensor_, false);
    this->publish_state_(this->discharging_binary_sensor_, true);
  }

  // Cycle charge
  float cycle_charge = seplos_get_32bit(8) * 0.01f;
  this->publish_state_(this->cycle_charge_sensor_, cycle_charge);

  // Pack count
  uint16_t pack_count = seplos_get_16bit(44);
  this->pack_count_ = std::min(pack_count, (uint16_t) 16);
  this->publish_state_(this->pack_count_sensor_, (float) this->pack_count_);

  // Cycles
  uint16_t cycles = seplos_get_16bit(46);
  this->publish_state_(this->charging_cycles_sensor_, (float) cycles);

  // Battery level
  this->publish_state_(this->state_of_charge_sensor_, seplos_get_16bit(48) * 0.1f);

  // State of Health
  this->publish_state_(this->state_of_health_sensor_, seplos_get_16bit(50) * 0.1f);

  // Remaining capacity - EIA register 0x2004
  float remaining_capacity = seplos_get_32bit(16) * 0.01f;
  this->publish_state_(this->capacity_remaining_sensor_, remaining_capacity);

  // Total capacity - EIA register 0x2006
  this->publish_state_(this->total_capacity_sensor_, seplos_get_32bit(20) * 0.01f);

  // Rated capacity - EIA register 0x200A
  this->publish_state_(this->rated_capacity_sensor_, seplos_get_32bit(40) * 0.01f);

  // Max discharge current - EIA register 0x2010
  this->publish_state_(this->max_discharge_current_sensor_, seplos_get_32bit(32) * 0.1f);

  // Max charge current - EIA register 0x2012
  this->publish_state_(this->max_charge_current_sensor_, seplos_get_32bit(36) * 0.1f);

  // Calculate derived values
  if (cycles > 0) {
    float cycle_capacity = cycle_charge / cycles;
    this->publish_state_(this->cycle_capacity_sensor_, cycle_capacity);
  }

  // Use the actual remaining capacity from register instead of calculation
  // The real remaining capacity is now read from EIA/PIA registers above

  if (current < 0 && remaining_capacity > 0) {
    float runtime = remaining_capacity / (-current);
    this->publish_state_(this->runtime_sensor_, runtime);
  }
}

void SeplosBmsV3Ble::decode_eib_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGD(TAG, "Decoding EIB data (%d bytes)", data.size());

  // Max Cell Voltage - EIB register 0x2100
  uint16_t max_cell_voltage = seplos_get_16bit(0);
  this->publish_state_(this->max_cell_voltage_sensor_, max_cell_voltage * 0.001f);

  // Min Cell Voltage - EIB register 0x2101
  uint16_t min_cell_voltage = seplos_get_16bit(2);
  this->publish_state_(this->min_cell_voltage_sensor_, min_cell_voltage * 0.001f);

  // Max Cell Voltage ID - EIB register 0x2102
  this->publish_state_(this->max_voltage_cell_sensor_, (float) seplos_get_16bit(4));

  // Min Cell Voltage ID - EIB register 0x2103
  this->publish_state_(this->min_voltage_cell_sensor_, (float) seplos_get_16bit(6));

  // Max Pack Voltage - EIB register 0x2104
  this->publish_state_(this->max_pack_voltage_sensor_, seplos_get_16bit(8) * 0.01f);

  // Min Pack Voltage - EIB register 0x2105
  this->publish_state_(this->min_pack_voltage_sensor_, seplos_get_16bit(10) * 0.01f);

  // Max Pack Voltage ID - EIB register 0x2106
  this->publish_state_(this->max_pack_voltage_id_sensor_, (float) seplos_get_16bit(12));

  // Min Pack Voltage ID - EIB register 0x2107
  this->publish_state_(this->min_pack_voltage_id_sensor_, (float) seplos_get_16bit(14));

  // Max Cell Temperature - EIB register 0x2108
  this->publish_state_(this->max_cell_temperature_sensor_, (float) (int16_t) seplos_get_16bit(16));

  // Min Cell Temperature - EIB register 0x2109
  this->publish_state_(this->min_cell_temperature_sensor_, (float) (int16_t) seplos_get_16bit(18));

  // Average cell temperature - EIB register 0x210A
  this->publish_state_(this->average_cell_temperature_sensor_, (float) (int16_t) seplos_get_16bit(20));

  // Max Cell Temperature ID - EIB register 0x210B
  this->publish_state_(this->max_temperature_cell_sensor_, (float) seplos_get_16bit(22));

  // Min Cell Temperature ID - EIB register 0x210C
  this->publish_state_(this->min_temperature_cell_sensor_, (float) seplos_get_16bit(24));

  // Calculate delta voltage from EIB max/min values
  float delta_voltage = (max_cell_voltage * 0.001f) - (min_cell_voltage * 0.001f);
  this->publish_state_(this->delta_voltage_sensor_, delta_voltage);
}

void SeplosBmsV3Ble::decode_eic_data_(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Decoding EIC data (%d bytes)", data.size());

  // Problem code (1-9, 64-bit)
  uint64_t problem_code = 0;
  for (int i = 1; i < 10; i++) {
    problem_code = (problem_code << 8) | data[i];
  }

  // Apply mask as per Python implementation
  problem_code = problem_code & 0xFFFF00FF00FF0000ULL;

  this->publish_state_(this->problem_code_sensor_, (float) problem_code);

  bool has_problem = (problem_code != 0);
  this->publish_state_(this->problem_text_sensor_, has_problem ? "Problem detected" : "No problems");
}

void SeplosBmsV3Ble::decode_via_data_(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Decoding VIA data (%d bytes)", data.size());

  // Factory Names - VIA register 0x1700
  std::string factory_name;
  for (int i = 0; i < 20 && i < data.size(); i++) {
    if (data[i] != 0) {
      factory_name += (char) data[i];
    }
  }
  this->publish_state_(this->factory_name_text_sensor_, factory_name);

  // Device Names - VIA register 0x170A
  std::string device_name;
  for (int i = 20; i < 40 && i < data.size(); i++) {
    if (data[i] != 0) {
      device_name += (char) data[i];
    }
  }
  this->publish_state_(this->device_name_text_sensor_, device_name);

  // Firmware Version - VIA register 0x1714
  std::string firmware_version;
  for (int i = 40; i < 44 && i < data.size(); i++) {
    if (data[i] != 0) {
      firmware_version += (char) data[i];
    }
  }
  this->publish_state_(this->firmware_version_text_sensor_, firmware_version);

  // BMS Serial Number - VIA register 0x1715
  std::string bms_serial_number;
  for (int i = 44; i < 74 && i < data.size(); i++) {
    if (data[i] != 0) {
      bms_serial_number += (char) data[i];
    }
  }
  this->publish_state_(this->bms_serial_number_text_sensor_, bms_serial_number);

  // Pack Serial Number - VIA register 0x1724
  std::string pack_serial_number;
  for (int i = 74; i < 104 && i < data.size(); i++) {
    if (data[i] != 0) {
      pack_serial_number += (char) data[i];
    }
  }
  this->publish_state_(this->pack_serial_number_text_sensor_, pack_serial_number);
}

bool SeplosBmsV3Ble::send_command_(uint8_t function, const std::vector<uint8_t> &payload) {
  if (this->node_state != espbt::ClientState::ESTABLISHED) {
    ESP_LOGW(TAG, "Not connected, cannot send command");
    return false;
  }

  ESP_LOGD(TAG, "Sending command 0x%02X with payload: %s", function, format_hex_pretty(payload).c_str());

  auto status = esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(),
                                         this->char_command_handle_, payload.size(), (uint8_t *) payload.data(),
                                         ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGW(TAG, "Error sending command, status=%d", status);
    return false;
  }

  return true;
}

std::vector<uint8_t> SeplosBmsV3Ble::build_modbus_payload_(const SeplosV3Command &cmd) {
  std::vector<uint8_t> payload;
  payload.push_back(cmd.device);
  payload.push_back(cmd.function);
  payload.push_back((cmd.reg_start >> 8) & 0xFF);
  payload.push_back(cmd.reg_start & 0xFF);

  uint16_t count = cmd.reg_count * (cmd.function == SEPLOS_V3_CMD_READ_01 ? 0x10 : 0x01);
  payload.push_back((count >> 8) & 0xFF);
  payload.push_back(count & 0xFF);

  uint16_t crc = this->crc16_(payload.data(), payload.size());
  payload.push_back(crc & 0xFF);
  payload.push_back((crc >> 8) & 0xFF);

  return payload;
}

uint16_t SeplosBmsV3Ble::crc16_(const uint8_t *data, uint16_t length) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc = crc >> 1;
      }
    }
  }
  return crc;
}

void SeplosBmsV3Ble::publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state) {
  if (binary_sensor != nullptr) {
    binary_sensor->publish_state(state);
  }
}

void SeplosBmsV3Ble::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor != nullptr && !std::isnan(value)) {
    sensor->publish_state(value);
  }
}

void SeplosBmsV3Ble::publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state) {
  if (text_sensor != nullptr) {
    text_sensor->publish_state(state);
  }
}

void SeplosBmsV3Ble::build_dynamic_command_queue_() {
  this->dynamic_command_queue_.clear();

  // Add system commands (always present)
  for (const auto &cmd : SEPLOS_V3_SYSTEM_COMMANDS) {
    this->dynamic_command_queue_.push_back(cmd);
  }

  // Add pack-specific commands only for registered pack sensors
  // This ensures commands are only sent to addresses that have corresponding pack components
  for (const auto *pack_sensor : this->pack_sensors_) {
    uint8_t pack_address = pack_sensor->get_address();
    ESP_LOGD(TAG, "Adding pack commands for registered address: 0x%02X", pack_address);

    for (const auto &pack_cmd : SEPLOS_V3_PACK_COMMANDS) {
      SeplosV3Command cmd = pack_cmd;
      cmd.device = pack_address;
      this->dynamic_command_queue_.push_back(cmd);
    }
  }

  ESP_LOGD(TAG, "Built dynamic command queue with %d commands for %d registered packs",
           this->dynamic_command_queue_.size(), this->pack_sensors_.size());
}

int SeplosBmsV3Ble::find_pack_index_by_address_(uint8_t address) {
  for (size_t i = 0; i < this->pack_sensors_.size(); i++) {
    if (this->pack_sensors_[i]->get_address() == address) {
      return i;
    }
  }
  return -1;
}

void SeplosBmsV3Ble::update_pack_data(uint8_t address, const std::vector<uint8_t> &data) {
  // Find the corresponding pack sensor
  for (auto *pack_sensor : this->pack_sensors_) {
    if (pack_sensor->get_address() == address) {
      ESP_LOGD(TAG, "Updating pack data for address: 0x%02X", address);

      // Determine data type based on data length and pass to pack sensor
      if (data.size() == SEPLOS_V3_PIA_LENGTH * 2) {
        // PIA data (Pack Information A)
        pack_sensor->decode_pia_data(data);
      } else if (data.size() == SEPLOS_V3_PIB_LENGTH * 2) {
        // PIB data (Pack Information B)
        pack_sensor->decode_pib_data(data);
      } else if (data.size() == SEPLOS_V3_PIC_LENGTH * 2) {
        // PIC data (Pack Information C)
        pack_sensor->decode_pic_data(data);
      } else {
        ESP_LOGW(TAG, "Unknown data format for pack 0x%02X, size: %d", address, data.size());
      }

      break;
    }
  }
}

void SeplosBmsV3Ble::update_pack_pia_data(uint8_t address, const std::vector<uint8_t> &data) {
  for (auto *pack_sensor : this->pack_sensors_) {
    if (pack_sensor->get_address() == address) {
      pack_sensor->decode_pia_data(data);
      break;
    }
  }
}

void SeplosBmsV3Ble::update_pack_pib_data(uint8_t address, const std::vector<uint8_t> &data) {
  for (auto *pack_sensor : this->pack_sensors_) {
    if (pack_sensor->get_address() == address) {
      pack_sensor->decode_pib_data(data);
      break;
    }
  }
}

void SeplosBmsV3Ble::update_pack_pic_data(uint8_t address, const std::vector<uint8_t> &data) {
  for (auto *pack_sensor : this->pack_sensors_) {
    if (pack_sensor->get_address() == address) {
      pack_sensor->decode_pic_data(data);
      break;
    }
  }
}

}  // namespace seplos_bms_v3_ble
}  // namespace esphome
