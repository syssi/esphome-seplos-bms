#include "seplos_bms_v3_ble.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/version.h"
#include "../seplos_bms_v3_ble_pack/seplos_bms_v3_ble_pack.h"
#include <cmath>

#if ESPHOME_VERSION_CODE >= VERSION_CODE(2025, 12, 0)
#define ADDR_STR(x) x
#else
#define ADDR_STR(x) (x).c_str()
#endif

namespace esphome::seplos_bms_v3_ble {

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
static const uint16_t SEPLOS_V3_REG_PIC_START = 0x1200;
static const uint16_t SEPLOS_V3_REG_VIA_START = 0x1700;
static const uint16_t SEPLOS_V3_REG_PCT_START = 0x1800;
static const uint16_t SEPLOS_V3_REG_SFA_START = 0x1400;
static const uint16_t SEPLOS_V3_REG_SPA1_START = 0x1300;
static const uint16_t SEPLOS_V3_REG_SPA2_START = 0x1335;

static const uint16_t SEPLOS_V3_EIA_LENGTH = 0x1A;
static const uint16_t SEPLOS_V3_EIB_LENGTH = 0x1A;
static const uint16_t SEPLOS_V3_EIC_LENGTH = 0x05;
static const uint16_t SEPLOS_V3_PIA_LENGTH = 0x11;
static const uint16_t SEPLOS_V3_PIB_LENGTH = 0x1A;
static const uint16_t SEPLOS_V3_PIC_LENGTH = 0x90;
static const uint16_t SEPLOS_V3_PCT_LENGTH = 0x24;
static const uint16_t SEPLOS_V3_SFA_LENGTH = 0x50;
static const uint16_t SEPLOS_V3_SPA_LENGTH = 0x35;
// Factory(10) + Device(10) + FW(1) + BMS_SN(15) + Pack_SN(15) = 51 registers
static const uint16_t SEPLOS_V3_VIA_LENGTH = 0x33;

static const SeplosV3Command SEPLOS_V3_SYSTEM_COMMANDS[] = {
    {0x00, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_EIA_START, SEPLOS_V3_EIA_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_EIB_START, SEPLOS_V3_EIB_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_01, SEPLOS_V3_REG_EIC_START, SEPLOS_V3_EIC_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_VIA_START, SEPLOS_V3_VIA_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_PCT_START, SEPLOS_V3_PCT_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_01, SEPLOS_V3_REG_SFA_START, SEPLOS_V3_SFA_LENGTH},
    {0xE0, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_SPA1_START, SEPLOS_V3_SPA_LENGTH},
    {0xE0, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_SPA2_START, SEPLOS_V3_SPA_LENGTH},
};

static const SeplosV3Command SEPLOS_V3_PACK_COMMANDS[] = {
    {0x00, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_PIA_START, SEPLOS_V3_PIA_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_04, SEPLOS_V3_REG_PIB_START, SEPLOS_V3_PIB_LENGTH},
    {0x00, SEPLOS_V3_CMD_READ_01, SEPLOS_V3_REG_PIC_START, SEPLOS_V3_PIC_LENGTH},
};

#ifdef USE_ESP32
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
                 ADDR_STR(this->parent_->address_str()));
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
                 ADDR_STR(this->parent_->address_str()));
        break;
      }
      this->char_command_handle_ = char_command->handle;
      break;
    }
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      this->node_state = espbt::ClientState::ESTABLISHED;
      this->publish_state_(this->online_status_binary_sensor_, true);

      if (!this->dynamic_command_queue_.empty()) {
        this->pending_reg_start_ = this->dynamic_command_queue_[0].reg_start;
        this->send_command_(this->dynamic_command_queue_[0].function,
                            this->build_modbus_payload_(this->dynamic_command_queue_[0]));
      }
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      ESP_LOGVV(TAG, "Notification received: %s",
                format_hex_pretty(param->notify.value, param->notify.value_len).c_str());  // NOLINT
      this->assemble(param->notify.value, param->notify.value_len);
      break;
    }
    default:
      break;
  }
}

#endif  // USE_ESP32

void SeplosBmsV3Ble::dump_config() {
  ESP_LOGCONFIG(TAG, "Seplos BMS V3 BLE");
  ESP_LOGCONFIG(TAG, "  Update interval: %dms", this->get_update_interval());
}

#ifdef USE_ESP32
void SeplosBmsV3Ble::update() {
  if (this->node_state != espbt::ClientState::ESTABLISHED) {
    ESP_LOGW(TAG, "[%s] Not connected", ADDR_STR(this->parent_->address_str()));
    return;
  }

  // Build command queue once when first update is called (all pack devices are registered by then)
  this->build_dynamic_command_queue_();

  if (this->next_command_ != 0) {
    ESP_LOGW(TAG, "Command queue (%d of %d) was not completely processed", this->next_command_,
             this->dynamic_command_queue_.size());
  }

  this->next_command_ = 0;
  if (!this->dynamic_command_queue_.empty()) {
    this->pending_reg_start_ = this->dynamic_command_queue_[0].reg_start;
    this->send_command_(this->dynamic_command_queue_[0].function,
                        this->build_modbus_payload_(this->dynamic_command_queue_[0]));
    this->next_command_++;
  }
}
#else
void SeplosBmsV3Ble::update() {}
#endif  // USE_ESP32

void SeplosBmsV3Ble::assemble(const uint8_t *data, uint16_t length) {
  if (this->frame_buffer_.size() > MAX_RESPONSE_SIZE) {
    ESP_LOGW(TAG, "Frame dropped because of invalid length");
    this->frame_buffer_.clear();
  }

  // Add incoming data to buffer
  this->frame_buffer_.insert(this->frame_buffer_.end(), data, data + length);

  // Check if we have a complete ModBus frame
  if (this->frame_buffer_.size() >= 5) {
    uint8_t function = this->frame_buffer_[1];
    uint16_t data_len = 0;

    if ((function & 0x80) == 0) {
      data_len = this->frame_buffer_[2];
    }

    uint16_t expected_length = 3 + data_len + 2;  // header + data + CRC

    if (this->frame_buffer_.size() >= expected_length) {
      // Verify CRC
      uint16_t frame_crc = this->frame_buffer_[expected_length - 2] | (this->frame_buffer_[expected_length - 1] << 8);
      uint16_t computed_crc = this->crc16_(this->frame_buffer_.data(), expected_length - 2);

      if (computed_crc == frame_crc) {
        std::vector<uint8_t> complete_frame(this->frame_buffer_.begin(), this->frame_buffer_.begin() + expected_length);
        this->decode(complete_frame);

#ifdef USE_ESP32
        // Send next command if available
        if (this->next_command_ < this->dynamic_command_queue_.size()) {
          this->pending_reg_start_ = this->dynamic_command_queue_[this->next_command_].reg_start;
          this->send_command_(this->dynamic_command_queue_[this->next_command_].function,
                              this->build_modbus_payload_(this->dynamic_command_queue_[this->next_command_]));
          this->next_command_++;
        }
#endif
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
  uint16_t data_len = data[2];

  ESP_LOGD(TAG, "Decoding frame: device=0x%02X, function=0x%02X, length=%d", device, function, data_len);

  if (function & 0x80) {
    ESP_LOGW(TAG, "Error response from device 0x%02X, error code: 0x%02X", device, data[2]);
    return;
  }

  std::vector<uint8_t> payload(data.begin() + 3, data.end() - 2);

  if (device == 0x00 || device == 0xE0) {
    switch (this->pending_reg_start_) {
      case SEPLOS_V3_REG_EIA_START:
        this->decode_eia_data_(payload);
        break;
      case SEPLOS_V3_REG_EIB_START:
        this->decode_eib_data_(payload);
        break;
      case SEPLOS_V3_REG_EIC_START:
        this->decode_eic_data_(payload);
        break;
      case SEPLOS_V3_REG_VIA_START:
        this->decode_via_data_(payload);
        break;
      case SEPLOS_V3_REG_PCT_START:
        this->decode_pct_data_(payload);
        break;
      case SEPLOS_V3_REG_SFA_START:
        this->decode_sfa_data_(payload);
        break;
      case SEPLOS_V3_REG_SPA1_START:
        this->decode_spa1_data_(payload);
        break;
      case SEPLOS_V3_REG_SPA2_START:
        this->decode_spa2_data_(payload);
        break;
      default:
        ESP_LOGW(TAG, "Unknown pending register start: 0x%04X (data_len=%d)", this->pending_reg_start_, data_len);
        break;
    }
  } else if (device >= 1 && device <= 16) {
    for (auto *pack_device : this->pack_devices_) {
      if (pack_device->get_address() == device) {
        pack_device->on_frame_data(data);
        return;
      }
    }
    ESP_LOGW(TAG, "No pack sensor found for address: 0x%02X", device);
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

  ESP_LOGD(TAG, "Decoding EIA data (%zu bytes)", data.size());

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

  // Remaining capacity (Reg 0x2004, 10mAH → Ah)
  float remaining_capacity = seplos_get_32bit(8) * 0.01f;
  this->publish_state_(this->capacity_remaining_sensor_, remaining_capacity);

  // Total capacity (Reg 0x2006, 10mAH → Ah)
  this->publish_state_(this->total_capacity_sensor_, seplos_get_32bit(12) * 0.01f);

  // Total discharge capacity = cumulative cycle charge (Reg 0x2008, 10AH → Ah)
  float cycle_charge = seplos_get_32bit(16) * 10.0f;
  this->publish_state_(this->cycle_charge_sensor_, cycle_charge);

  // Rated capacity (Reg 0x200A, 10mAH → Ah)
  this->publish_state_(this->rated_capacity_sensor_, seplos_get_32bit(20) * 0.01f);

  // Max discharge current (Reg 0x2010, 100mA → A)
  this->publish_state_(this->max_discharge_current_sensor_, seplos_get_32bit(32) * 0.1f);

  // Max charge current (Reg 0x2012, 100mA → A)
  this->publish_state_(this->max_charge_current_sensor_, seplos_get_32bit(36) * 0.1f);

  // Pack count (Reg 0x2016)
  uint16_t pack_count = seplos_get_16bit(44);
  this->pack_count_ = std::min(pack_count, (uint16_t) 16);
  this->publish_state_(this->pack_count_sensor_, (float) this->pack_count_);

  // Cycles (Reg 0x2017)
  uint16_t cycles = seplos_get_16bit(46);
  this->publish_state_(this->charging_cycles_sensor_, (float) cycles);

  // SOC (Reg 0x2018, 0.1% → %)
  this->publish_state_(this->state_of_charge_sensor_, seplos_get_16bit(48) * 0.1f);

  // SOH (Reg 0x2019, 0.1% → %)
  this->publish_state_(this->state_of_health_sensor_, seplos_get_16bit(50) * 0.1f);

  if (cycles > 0) {
    this->publish_state_(this->cycle_capacity_sensor_, cycle_charge / cycles);
  }

  if (current < 0 && remaining_capacity > 0) {
    this->publish_state_(this->runtime_sensor_, remaining_capacity / (-current));
  }
}

void SeplosBmsV3Ble::decode_eib_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGD(TAG, "Decoding EIB data (%zu bytes)", data.size());

  // Debug: Print raw hex data
  std::string hex_str;
  for (size_t i = 0; i < std::min((size_t) 32, data.size()); i++) {
    char hex[4];
    sprintf(hex, "%02X ", data[i]);  // NOLINT
    hex_str += hex;
  }
  ESP_LOGD(TAG, "EIB raw data (first 32 bytes): %s", hex_str.c_str());

  // Max Cell Voltage - EIB register 0x2100
  uint16_t max_cell_voltage = seplos_get_16bit(0);
  ESP_LOGD(TAG, "Max cell voltage raw: 0x%04X (%d), calculated: %.3fV", max_cell_voltage, max_cell_voltage,
           max_cell_voltage * 0.001f);
  this->publish_state_(this->max_cell_voltage_sensor_, max_cell_voltage * 0.001f);

  // Min Cell Voltage - EIB register 0x2101
  uint16_t min_cell_voltage = seplos_get_16bit(2);
  ESP_LOGD(TAG, "Min cell voltage raw: 0x%04X (%d), calculated: %.3fV", min_cell_voltage, min_cell_voltage,
           min_cell_voltage * 0.001f);
  this->publish_state_(this->min_cell_voltage_sensor_, min_cell_voltage * 0.001f);

  // Max Cell Voltage ID - EIB register 0x2102
  uint16_t max_voltage_cell_id = seplos_get_16bit(4);
  ESP_LOGD(TAG, "Max voltage cell ID raw: 0x%04X (%d)", max_voltage_cell_id, max_voltage_cell_id);
  this->publish_state_(this->max_voltage_cell_sensor_, (float) max_voltage_cell_id);

  // Min Cell Voltage ID - EIB register 0x2103
  uint16_t min_voltage_cell_id = seplos_get_16bit(6);
  ESP_LOGD(TAG, "Min voltage cell ID raw: 0x%04X (%d)", min_voltage_cell_id, min_voltage_cell_id);
  this->publish_state_(this->min_voltage_cell_sensor_, (float) min_voltage_cell_id);

  // Max Pack Voltage - EIB register 0x2104
  this->publish_state_(this->max_pack_voltage_sensor_, seplos_get_16bit(8) * 0.01f);

  // Min Pack Voltage - EIB register 0x2105
  uint16_t min_pack_voltage = seplos_get_16bit(10);
  ESP_LOGD(TAG, "Min pack voltage raw: 0x%04X (%d), calculated: %.2fV", min_pack_voltage, min_pack_voltage,
           min_pack_voltage * 0.01f);
  this->publish_state_(this->min_pack_voltage_sensor_, min_pack_voltage * 0.01f);

  // Max Pack Voltage ID - EIB register 0x2106
  uint16_t max_pack_voltage_id = seplos_get_16bit(12);
  ESP_LOGD(TAG, "Max pack voltage ID raw: 0x%04X (%d)", max_pack_voltage_id, max_pack_voltage_id);
  this->publish_state_(this->max_pack_voltage_id_sensor_, (float) max_pack_voltage_id);

  // Min Pack Voltage ID - EIB register 0x2107
  uint16_t min_pack_voltage_id = seplos_get_16bit(14);
  ESP_LOGD(TAG, "Min pack voltage ID raw: 0x%04X (%d)", min_pack_voltage_id, min_pack_voltage_id);
  this->publish_state_(this->min_pack_voltage_id_sensor_, (float) min_pack_voltage_id);

  // Max Cell Temperature - EIB register 0x2108 (INT16, 0.1K format like BMS_BLE-HA)
  int16_t max_cell_temp_raw = (int16_t) seplos_get_16bit(16);
  float max_cell_temp = (max_cell_temp_raw - 2731.5f) / 10.0f;  // Convert from 0.1K to °C
  ESP_LOGD(TAG, "Max cell temp raw: 0x%04X (%d), as 0.1K format: %.1f°C", seplos_get_16bit(16), max_cell_temp_raw,
           max_cell_temp);
  this->publish_state_(this->max_cell_temperature_sensor_, max_cell_temp);

  // Min Cell Temperature - EIB register 0x2109 (INT16, 0.1K format like BMS_BLE-HA)
  int16_t min_cell_temp_raw = (int16_t) seplos_get_16bit(18);
  float min_cell_temp = (min_cell_temp_raw - 2731.5f) / 10.0f;  // Convert from 0.1K to °C
  ESP_LOGD(TAG, "Min cell temp raw: 0x%04X (%d), as 0.1K format: %.1f°C", seplos_get_16bit(18), min_cell_temp_raw,
           min_cell_temp);
  this->publish_state_(this->min_cell_temperature_sensor_, min_cell_temp);

  // Average cell temperature - EIB register 0x210A (INT16, 0.1K format like BMS_BLE-HA)
  int16_t avg_cell_temp_raw = (int16_t) seplos_get_16bit(20);
  float avg_cell_temp = (avg_cell_temp_raw - 2731.5f) / 10.0f;  // Convert from 0.1K to °C
  ESP_LOGD(TAG, "Avg cell temp raw: 0x%04X (%d), as 0.1K format: %.1f°C", seplos_get_16bit(20), avg_cell_temp_raw,
           avg_cell_temp);
  this->publish_state_(this->average_cell_temperature_sensor_, avg_cell_temp);

  // Max Cell Temperature ID - EIB register 0x210B
  uint16_t max_temp_cell_id = seplos_get_16bit(22);
  ESP_LOGD(TAG, "Max temp cell ID raw: 0x%04X (%d)", max_temp_cell_id, max_temp_cell_id);
  this->publish_state_(this->max_temperature_cell_sensor_, (float) max_temp_cell_id);

  // Min Cell Temperature ID - EIB register 0x210C
  uint16_t min_temp_cell_id = seplos_get_16bit(24);
  ESP_LOGD(TAG, "Min temp cell ID raw: 0x%04X (%d)", min_temp_cell_id, min_temp_cell_id);
  this->publish_state_(this->min_temperature_cell_sensor_, (float) min_temp_cell_id);

  // Calculate delta voltage from EIB max/min values
  float delta_voltage = (max_cell_voltage * 0.001f) - (min_cell_voltage * 0.001f);
  this->publish_state_(this->delta_voltage_sensor_, delta_voltage);
}

void SeplosBmsV3Ble::decode_eic_data_(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Decoding EIC data (%zu bytes)", data.size());

  // Problem code (1-9, 64-bit)
  uint64_t problem_code = 0;
  for (uint8_t i = 1; i < 10; i++) {
    problem_code = (problem_code << 8) | data[i];
  }

  problem_code = problem_code & 0xFFFF00FF00FF0000ULL;

  this->publish_state_(this->problem_code_sensor_, (float) problem_code);

  bool has_problem = (problem_code != 0);
  this->publish_state_(this->problem_text_sensor_, has_problem ? "Problem detected" : "No problems");
}

void SeplosBmsV3Ble::decode_via_data_(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Decoding VIA data (Version Info) - %zu bytes", data.size());

  if (data.size() < 102) {
    ESP_LOGW(TAG, "VIA data too short: %zu bytes (expected 102)", data.size());
    return;
  }

  // The spec notes "fill in the small end first" for VIA, suggesting little-endian
  // byte order within each 16-bit register. We log raw hex so byte order can be
  // verified against a real device response.
  auto extract_string = [&](size_t offset, size_t len) -> std::string {
    std::string s(data.begin() + offset, data.begin() + offset + len);
    s.erase(std::find(s.begin(), s.end(), '\0'), s.end());
    return s;
  };

  // Reg 0x1700–0x1709: Factory Name (20 bytes)
  std::string factory_name = extract_string(0, 20);
  ESP_LOGI(TAG, "  Factory Name:     '%s'", factory_name.c_str());
  ESP_LOGD(TAG, "  Factory Name hex: %s", format_hex_pretty(data.data(), 20).c_str());  // NOLINT
  this->publish_state_(this->factory_name_text_sensor_, factory_name);

  // Reg 0x170A–0x1713: Device Name (20 bytes)
  std::string device_name = extract_string(20, 20);
  ESP_LOGI(TAG, "  Device Name:      '%s'", device_name.c_str());
  ESP_LOGD(TAG, "  Device Name hex:  %s", format_hex_pretty(data.data() + 20, 20).c_str());  // NOLINT
  this->publish_state_(this->device_name_text_sensor_, device_name);

  // Reg 0x1714: Firmware Version (2 bytes)
  std::string firmware_version = extract_string(40, 2);
  ESP_LOGI(TAG, "  Firmware Version: '%s'", firmware_version.c_str());
  this->publish_state_(this->firmware_version_text_sensor_, firmware_version);

  // Reg 0x1715–0x1723: BMS SN (30 bytes)
  std::string bms_serial = extract_string(42, 30);
  ESP_LOGI(TAG, "  BMS SN:           '%s'", bms_serial.c_str());
  ESP_LOGD(TAG, "  BMS SN hex:       %s", format_hex_pretty(data.data() + 42, 30).c_str());  // NOLINT
  this->publish_state_(this->bms_serial_number_text_sensor_, bms_serial);

  // Reg 0x1724–0x1732: Pack SN (30 bytes)
  std::string pack_serial = extract_string(72, 30);
  ESP_LOGI(TAG, "  Pack SN:          '%s'", pack_serial.c_str());
  ESP_LOGD(TAG, "  Pack SN hex:      %s", format_hex_pretty(data.data() + 72, 30).c_str());  // NOLINT
  this->publish_state_(this->pack_serial_number_text_sensor_, pack_serial);
}

void SeplosBmsV3Ble::decode_pct_data_(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Decoding PCT data (Protocol Control Type) - %zu bytes", data.size());

  // Length field (byte 2)
  uint8_t length = data[2];
  ESP_LOGD(TAG, "  Length: %d", length);

  // PCS Protocol type Switch (bytes 3-4)
  uint16_t protocol_type = (data[3] << 8) | data[4];
  ESP_LOGD(TAG, "  Protocol Type Switch: 0x%04X (%d)", protocol_type, protocol_type);

  // PCS baud rate (bytes 5-6)
  uint16_t baud_rate = (data[5] << 8) | data[6];
  ESP_LOGD(TAG, "  Baud Rate: %d", baud_rate);

  // PCS name (bytes 7-38) - 32 ASCII characters
  std::string pcs_name(data.begin() + 7, data.begin() + 39);
  pcs_name.erase(std::find(pcs_name.begin(), pcs_name.end(), '\0'), pcs_name.end());
  ESP_LOGD(TAG, "  PCS Name: '%s'", pcs_name.c_str());

  // Protocol support name (bytes 39-70) - 32 ASCII characters
  std::string protocol_name(data.begin() + 39, data.begin() + 71);
  protocol_name.erase(std::find(protocol_name.begin(), protocol_name.end(), '\0'), protocol_name.end());
  ESP_LOGD(TAG, "  Protocol Support Name: '%s'", protocol_name.c_str());

  // Protocol version (bytes 71-72) - 2 ASCII characters
  std::string protocol_version(data.begin() + 71, data.begin() + 73);
  protocol_version.erase(std::find(protocol_version.begin(), protocol_version.end(), '\0'), protocol_version.end());
  ESP_LOGD(TAG, "  Protocol Version: '%s'", protocol_version.c_str());

  // Protocol pre Switch (bytes 73-74)
  uint16_t protocol_pre_switch = (data[73] << 8) | data[74];
  ESP_LOGD(TAG, "  Protocol Pre Switch: 0x%04X (%d)", protocol_pre_switch, protocol_pre_switch);
}

void SeplosBmsV3Ble::decode_sfa_data_(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Decoding SFA data (System Function Switches) - %zu bytes", data.size());

  if (data.size() < 20) {
    ESP_LOGW(TAG, "SFA data too short: %zu bytes", data.size());
    return;
  }

  ESP_LOGD(TAG, "  Voltage Function Switch: 0x%02X", data[3]);
  ESP_LOGD(TAG, "  Cell Temperature Switch: 0x%02X", data[4]);
  ESP_LOGD(TAG, "  Environment/Power Temperature Switch: 0x%02X", data[5]);
  ESP_LOGD(TAG, "  General Function Switch: 0x%02X", data[6]);
  ESP_LOGD(TAG, "  Current Function Switch 1: 0x%02X", data[7]);
  ESP_LOGD(TAG, "  Current Function Switch 2: 0x%02X", data[8]);
  ESP_LOGD(TAG, "  Capacity/Other Switch: 0x%02X", data[9]);
  ESP_LOGD(TAG, "  Equalization Switch: 0x%02X", data[10]);
  ESP_LOGD(TAG, "  Indicator Switch: 0x%02X", data[11]);
  ESP_LOGD(TAG, "  Hard Fault Switch: 0x%02X", data[12]);
}

// SPA (System Parameters, registers 0x1300–0x1367, see "XZH BMS Modbus-RTU Protocol")
// is fetched in two requests (0x1300 and 0x1335), each a full SEPLOS_V3_SPA_LENGTH
// register block. Values are plain big-endian UINT16.

void SeplosBmsV3Ble::decode_spa1_data_(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Decoding SPA data (System Parameters, 0x1300) - %zu bytes", data.size());

  if (data.size() < SEPLOS_V3_SPA_LENGTH * 2) {
    ESP_LOGW(TAG, "SPA data too short: %zu bytes", data.size());
    return;
  }

  auto reg = [&](uint16_t addr) -> uint16_t {
    size_t offset = (size_t) (addr - SEPLOS_V3_REG_SPA1_START) * 2;
    return (uint16_t(data[offset]) << 8) | uint16_t(data[offset + 1]);
  };
  auto temperature = [&](uint16_t addr) -> float { return (reg(addr) - 2731.5f) * 0.1f; };

  ESP_LOGD(TAG, "NTC Count: %u", reg(0x1300));
  ESP_LOGD(TAG, "Cell Count: %u", reg(0x1301));
  ESP_LOGD(TAG, "Pack Overvoltage Recover: %.2f V", reg(0x1304) * 0.01f);
  ESP_LOGD(TAG, "Pack Overvoltage Protection: %.2f V", reg(0x1305) * 0.01f);
  ESP_LOGD(TAG, "Pack Undervoltage Recover: %.2f V", reg(0x1308) * 0.01f);
  ESP_LOGD(TAG, "Pack Undervoltage Protection: %.2f V", reg(0x1309) * 0.01f);
  ESP_LOGD(TAG, "Cell Overvoltage Recover: %u mV", reg(0x130C));
  ESP_LOGD(TAG, "Cell Overvoltage Protection: %u mV", reg(0x130D));
  ESP_LOGD(TAG, "Cell Undervoltage Recover: %u mV", reg(0x1310));
  ESP_LOGD(TAG, "Cell Undervoltage Protection: %u mV", reg(0x1311));
  ESP_LOGD(TAG, "Cell Difference Protection: %u mV", reg(0x1313));
  ESP_LOGD(TAG, "Charge Overcurrent Protection: %d A", reg(0x1317));
  ESP_LOGD(TAG, "Discharge Overcurrent Protection: %d A", (int16_t) reg(0x131D));
  ESP_LOGD(TAG, "Charge Overtemperature Protection: %.1f °C", temperature(0x1332));
  ESP_LOGD(TAG, "Charge Low Temperature Alarm: %.1f °C", temperature(0x1334));
}

void SeplosBmsV3Ble::decode_spa2_data_(const std::vector<uint8_t> &data) {
  ESP_LOGD(TAG, "Decoding SPA data (System Parameters, 0x1335) - %zu bytes", data.size());

  if (data.size() < SEPLOS_V3_SPA_LENGTH * 2) {
    ESP_LOGW(TAG, "SPA data too short: %zu bytes", data.size());
    return;
  }

  auto reg = [&](uint16_t addr) -> uint16_t {
    size_t offset = (size_t) (addr - SEPLOS_V3_REG_SPA2_START) * 2;
    return (uint16_t(data[offset]) << 8) | uint16_t(data[offset + 1]);
  };
  auto temperature = [&](uint16_t addr) -> float { return (reg(addr) - 2731.5f) * 0.1f; };

  int16_t discharge_limit = (int16_t) reg(0x1367);

  ESP_LOGD(TAG, "Discharge Overtemperature Protection: %.1f °C", temperature(0x133A));
  ESP_LOGD(TAG, "Under Environment Temperature Protection: %.1f °C", temperature(0x1346));
  ESP_LOGD(TAG, "Over Power Temperature Protection: %.1f °C", temperature(0x134A));
  ESP_LOGD(TAG, "Balancing Open Voltage: %u mV", reg(0x1350));
  ESP_LOGD(TAG, "Balancing Open Difference: %u mV", reg(0x1351));
  ESP_LOGD(TAG, "SOC Low Alarm: %.1f %%", reg(0x1355) * 0.1f);
  ESP_LOGD(TAG, "Rated Capacity: %.2f Ah", reg(0x1358) * 0.01f);
  ESP_LOGD(TAG, "Total Capacity: %.2f Ah", reg(0x1359) * 0.01f);
  ESP_LOGD(TAG, "PCS Charge Current Limit: %u A", reg(0x1366));
  ESP_LOGD(TAG, "PCS Discharge Current Limit: %d A", discharge_limit < 0 ? -discharge_limit : discharge_limit);
}

#ifdef USE_ESP32
bool SeplosBmsV3Ble::send_command_(uint8_t function, const std::vector<uint8_t> &payload) {
  if (this->node_state != espbt::ClientState::ESTABLISHED) {
    ESP_LOGW(TAG, "Not connected, cannot send command");
    return false;
  }

  ESP_LOGD(TAG, "Sending command 0x%02X with payload: %s", function, format_hex_pretty(payload).c_str());  // NOLINT

  auto status = esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(),
                                         this->char_command_handle_, payload.size(), (uint8_t *) payload.data(),
                                         ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);
  if (status) {
    ESP_LOGW(TAG, "Error sending command, status=%d", status);
    return false;
  }

  return true;
}
#endif  // USE_ESP32

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
  if (sensor != nullptr) {
    sensor->publish_state(value);
  }
}

void SeplosBmsV3Ble::publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state) {
  if (text_sensor != nullptr) {
    text_sensor->publish_state(state);
  }
}

void SeplosBmsV3Ble::build_dynamic_command_queue_() {
  if (!this->dynamic_command_queue_.empty()) {
    ESP_LOGD(TAG, "Command queue already built with %zu commands, skipping rebuild",
             this->dynamic_command_queue_.size());
    return;
  }

  // Add system commands (always present)
  for (const auto &cmd : SEPLOS_V3_SYSTEM_COMMANDS) {
    this->dynamic_command_queue_.push_back(cmd);
  }

  // Add pack-specific commands only for registered pack sensors
  // This ensures commands are only sent to addresses that have corresponding pack components
  for (const auto *pack_device : this->pack_devices_) {
    uint8_t pack_address = pack_device->get_address();
    ESP_LOGD(TAG, "Adding pack commands for registered address: 0x%02X", pack_address);

    for (const auto &pack_cmd : SEPLOS_V3_PACK_COMMANDS) {
      SeplosV3Command cmd = pack_cmd;
      cmd.device = pack_address;
      this->dynamic_command_queue_.push_back(cmd);
    }
  }

  ESP_LOGD(TAG, "Built dynamic command queue with %zu commands for %zu registered packs",
           this->dynamic_command_queue_.size(), this->pack_devices_.size());
}

}  // namespace esphome::seplos_bms_v3_ble
