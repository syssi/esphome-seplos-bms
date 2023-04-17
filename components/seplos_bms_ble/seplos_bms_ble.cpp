#include "seplos_bms_ble.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace seplos_bms_ble {

static const char *const TAG = "seplos_bms_ble";

static const uint16_t SEPLOS_BMS_SERVICE_UUID = 0xFF00;
static const uint16_t SEPLOS_BMS_NOTIFY_CHARACTERISTIC_UUID = 0xFF01;   // handle 0x12
static const uint16_t SEPLOS_BMS_CONTROL_CHARACTERISTIC_UUID = 0xFF02;  // handle 0x14

static const uint16_t MAX_RESPONSE_SIZE = 20;

static const uint8_t SEPLOS_PKT_START = 0x55;
static const uint8_t SEPLOS_FUNCTION_REQUEST = 0x04;
static const uint8_t SEPLOS_FUNCTION_RESPONSE = 0x14;
static const uint8_t SEPLOS_PKT_END = 0xAA;

static const uint8_t SEPLOS_FRAME_TYPE_SOFTWARE_VERSION = 0x81;
static const uint8_t SEPLOS_FRAME_TYPE_HARDWARE_VERSION = 0x82;
static const uint8_t SEPLOS_FRAME_TYPE_STATUS = 0x83;
static const uint8_t SEPLOS_FRAME_TYPE_GENERAL_INFO = 0x84;
static const uint8_t SEPLOS_FRAME_TYPE_MOSFET_STATUS = 0x85;
static const uint8_t SEPLOS_FRAME_TYPE_GYRO = 0x86;
static const uint8_t SEPLOS_FRAME_TYPE_TEMPERATURES = 0x87;
static const uint8_t SEPLOS_FRAME_TYPE_CELL_VOLTAGES_1_8 = 0x88;
static const uint8_t SEPLOS_FRAME_TYPE_CELL_VOLTAGES_9_16 = 0x89;
static const uint8_t SEPLOS_FRAME_TYPE_CELL_VOLTAGES_17_24 = 0x8A;
static const uint8_t SEPLOS_FRAME_TYPE_LOCATION = 0x90;
static const uint8_t SEPLOS_FRAME_TYPE_OWNER = 0x94;

static const uint8_t SEPLOS_COMMAND_QUEUE_SIZE = 6;
static const uint8_t SEPLOS_COMMAND_QUEUE[SEPLOS_COMMAND_QUEUE_SIZE] = {
    SEPLOS_FRAME_TYPE_STATUS,       SEPLOS_FRAME_TYPE_GENERAL_INFO,      SEPLOS_FRAME_TYPE_MOSFET_STATUS,
    SEPLOS_FRAME_TYPE_TEMPERATURES, SEPLOS_FRAME_TYPE_CELL_VOLTAGES_1_8, SEPLOS_FRAME_TYPE_CELL_VOLTAGES_9_16,
    // SEPLOS_FRAME_TYPE_CELL_VOLTAGES_17_24,
};

static const uint8_t VOLTAGE_PROTECTION_ERRORS_SIZE = 16;
static const char *const VOLTAGE_PROTECTION_ERRORS[VOLTAGE_PROTECTION_ERRORS_SIZE] = {
    "Cell overvoltage protection",         // 0000 0000 0000 0001
    "Cell undervoltage protection",        // 0000 0000 0000 0010
    "Pack overvoltage protection",         // 0000 0000 0000 0100
    "Pack undervoltage protection",        // 0000 0000 0000 1000
    "Cell overvoltage alarm",              // 0000 0000 0001 0000
    "Cell undervoltage alarm",             // 0000 0000 0010 0000
    "Pack overvoltage alarm",              // 0000 0000 0100 0000
    "Pack undervoltage alarm",             // 0000 0000 1000 0000
    "Cell voltage difference alarm",       // 0000 0001 0000 0000
    "Reserved",                            // 0000 0010 0000 0000
    "Reserved",                            // 0000 0100 0000 0000
    "Reserved",                            // 0000 1000 0000 0000
    "Reserved",                            // 0001 0000 0000 0000
    "Reserved",                            // 0010 0000 0000 0000
    "The system is going to sleep state",  // 0100 0000 0000 0000
    "Reserved",                            // 1000 0000 0000 0000
};

static const uint8_t TEMPERATURE_PROTECTION_ERRORS_SIZE = 16;
static const char *const TEMPERATURE_PROTECTION_ERRORS[TEMPERATURE_PROTECTION_ERRORS_SIZE] = {
    "Charge over temperature protection",      // 0000 0000 0000 0001
    "Charge under temperature protection",     // 0000 0000 0000 0010
    "Discharge over temperature protection",   // 0000 0000 0000 0100
    "Discharge under temperature protection",  // 0000 0000 0000 1000
    "Ambient over temperature protection",     // 0000 0000 0001 0000
    "Ambient under temperature protection",    // 0000 0000 0010 0000
    "MOS over temperature protection",         // 0000 0000 0100 0000
    "MOS under temperature protection",        // 0000 0000 1000 0000
    "Charge over temperature alarm",           // 0000 0001 0000 0000
    "Charge under temperature alarm",          // 0000 0010 0000 0000
    "Discharge over temperature alarm",        // 0000 0100 0000 0000
    "Discharge under temperature alarm",       // 0000 1000 0000 0000
    "Ambient over temperature alarm",          // 0001 0000 0000 0000
    "Ambient under temperature alarm",         // 0010 0000 0000 0000
    "MOS over temperature alarm",              // 0100 0000 0000 0000
    "MOS under temperature alarm",             // 1000 0000 0000 0000
};

static const uint8_t CURRENT_PROTECTION_ERRORS_SIZE = 16;
static const char *const CURRENT_PROTECTION_ERRORS[CURRENT_PROTECTION_ERRORS_SIZE] = {
    "Charge overcurrent protection",       // 0000 0000 0000 0001
    "Short circuit protection",            // 0000 0000 0000 0010
    "Discharge overcurrent 1 protection",  // 0000 0000 0000 0100
    "Discharge overcurrent 2 protection",  // 0000 0000 0000 1000
    "Charge overcurrent alarm",            // 0000 0000 0001 0000
    "Discharge overcurrent alarm",         // 0000 0000 0010 0000
    "Gyro lock alarm",                     // 0000 0000 0100 0000
    "Reserved",                            // 0000 0000 1000 0000
    "Reserved",                            // 0000 0001 0000 0000
    "Reserved",                            // 0000 0010 0000 0000
    "Reserved",                            // 0000 0100 0000 0000
    "Reserved",                            // 0000 1000 0000 0000
    "Reserved",                            // 0001 0000 0000 0000
    "Reserved",                            // 0010 0000 0000 0000
    "Reserved",                            // 0100 0000 0000 0000
    "Reserved",                            // 1000 0000 0000 0000
};

static const uint8_t ERRORS_SIZE = 16;
static const char *const ERRORS[ERRORS_SIZE] = {
    "Cell voltage differential alarm",  // 0000 0000 0000 0001
    "Charge MOS damage alarm",          // 0000 0000 0000 0010
    "External SD card failure alarm",   // 0000 0000 0000 0100
    "SPI communication failure alarm",  // 0000 0000 0000 1000
    "EEPROM failure alarm",             // 0000 0000 0001 0000
    "LED alarm enable",                 // 0000 0000 0010 0000
    "Buzzer alarm enable",              // 0000 0000 0100 0000
    "Low battery alarm",                // 0000 0000 1000 0000
    "MOS over temperature protection",  // 0000 0001 0000 0000
    "MOS over temperature alarm",       // 0000 0010 0000 0000
    "Current limiting board failure",   // 0000 0100 0000 0000
    "Sampling failure",                 // 0000 1000 0000 0000
    "Battery failure",                  // 0001 0000 0000 0000
    "NTC failure",                      // 0010 0000 0000 0000
    "Charge MOS failure",               // 0100 0000 0000 0000
    "Discharge MOS failure",            // 1000 0000 0000 0000
};

void SeplosBmsBle::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                       esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      this->node_state = espbt::ClientState::IDLE;

      // this->publish_state_(this->voltage_sensor_, NAN);
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      // [esp32_ble_client:048]: [0] [60:6E:41:FF:FF:FF] Found device
      // [esp32_ble_client:064]: [0] [60:6E:41:FF:FF:FF] 0x00 Attempting BLE connection
      // [esp32_ble_client:192]: [0] [60:6E:41:FF:FF:FF] Service UUID: 0x1801
      // [esp32_ble_client:194]: [0] [60:6E:41:FF:FF:FF]  start_handle: 0x1  end_handle: 0x3
      // [esp32_ble_client:192]: [0] [60:6E:41:FF:FF:FF] Service UUID: 0x1800
      // [esp32_ble_client:194]: [0] [60:6E:41:FF:FF:FF]  start_handle: 0x4  end_handle: 0xe
      // [esp32_ble_client:192]: [0] [60:6E:41:FF:FF:FF] Service UUID: 0xFF00
      // [esp32_ble_client:194]: [0] [60:6E:41:FF:FF:FF]  start_handle: 0xf  end_handle: 0x1a
      // [esp32_ble_client:196]: [0] [60:6E:41:FF:FF:FF] Connected
      // [esp32_ble_client:114]: [0] [60:6E:41:FF:FF:FF] gattc_event_handler: event=18 gattc_if=3
      // [esp32_ble_client:160]: [0] [60:6E:41:FF:FF:FF] cfg_mtu status 0, mtu 51

      auto *char_notify =
          this->parent_->get_characteristic(SEPLOS_BMS_SERVICE_UUID, SEPLOS_BMS_NOTIFY_CHARACTERISTIC_UUID);
      if (char_notify == nullptr) {
        ESP_LOGE(TAG, "[%s] No notify service found at device, not an Seplos BMS..?",
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
          this->parent_->get_characteristic(SEPLOS_BMS_SERVICE_UUID, SEPLOS_BMS_CONTROL_CHARACTERISTIC_UUID);
      if (char_command == nullptr) {
        ESP_LOGE(TAG, "[%s] No control service found at device, not an BASEN BMS..?",
                 this->parent_->address_str().c_str());
        break;
      }
      this->char_command_handle_ = char_command->handle;
      break;
    }
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      this->node_state = espbt::ClientState::ESTABLISHED;

      this->send_command_(SEPLOS_FRAME_TYPE_SOFTWARE_VERSION);
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      ESP_LOGV(TAG, "Notification received (handle 0x%02X): %s", param->notify.handle,
               format_hex_pretty(param->notify.value, param->notify.value_len).c_str());

      // std::vector<uint8_t> data(param->notify.value, param->notify.value + param->notify.value_len);

      // this->on_seplos_bms_ble_data(param->notify.handle, data);
      break;
    }
    default:
      break;
  }
}

void SeplosBmsBle::update() {
  if (this->node_state != espbt::ClientState::ESTABLISHED) {
    ESP_LOGW(TAG, "[%s] Not connected", this->parent_->address_str().c_str());
    return;
  }

  // Loop through all commands if connected
  if (this->next_command_ != SEPLOS_COMMAND_QUEUE_SIZE) {
    ESP_LOGW(TAG,
             "Command queue (%d of %d) was not completely processed. "
             "Please increase the update_interval if you see this warning frequently",
             this->next_command_ + 1, SEPLOS_COMMAND_QUEUE_SIZE);
  }
  this->next_command_ = 0;
  this->send_command_(SEPLOS_COMMAND_QUEUE[this->next_command_++ % SEPLOS_COMMAND_QUEUE_SIZE]);
}

void SeplosBmsBle::on_seplos_bms_ble_data(const uint8_t &handle, const std::vector<uint8_t> &data) {
  if (data[0] != SEPLOS_PKT_START || data.back() != SEPLOS_PKT_END || data.size() != MAX_RESPONSE_SIZE) {
    ESP_LOGW(TAG, "Invalid response received: %s", format_hex_pretty(&data.front(), data.size()).c_str());
    return;
  }

  uint8_t frame_type = data[2];

  switch (frame_type) {
    case SEPLOS_FRAME_TYPE_SOFTWARE_VERSION:
      this->decode_software_version_data_(data);
      break;
    case SEPLOS_FRAME_TYPE_HARDWARE_VERSION:
      this->decode_hardware_version_data_(data);
      break;
    case SEPLOS_FRAME_TYPE_STATUS:
      this->decode_status_data_(data);
      break;
    case SEPLOS_FRAME_TYPE_GENERAL_INFO:
      this->decode_general_info_data_(data);
      break;
    case SEPLOS_FRAME_TYPE_MOSFET_STATUS:
      this->decode_mosfet_status_data_(data);
      break;
    case SEPLOS_FRAME_TYPE_TEMPERATURES:
      this->decode_temperature_data_(data);
      break;
    case SEPLOS_FRAME_TYPE_CELL_VOLTAGES_1_8:
    case SEPLOS_FRAME_TYPE_CELL_VOLTAGES_9_16:
    case SEPLOS_FRAME_TYPE_CELL_VOLTAGES_17_24:
      this->decode_cell_voltages_data_(frame_type - SEPLOS_FRAME_TYPE_CELL_VOLTAGES_1_8, data);
      break;
    case SEPLOS_FRAME_TYPE_LOCATION:
    case SEPLOS_FRAME_TYPE_LOCATION + 1:
      ESP_LOGD(TAG, "The location frame isn't supported yet");
      break;
    case SEPLOS_FRAME_TYPE_OWNER:
    case SEPLOS_FRAME_TYPE_OWNER + 1:
      ESP_LOGD(TAG, "The owner frame isn't supported yet");
      break;
    default:
      ESP_LOGW(TAG, "Unhandled response received (frame_type 0x%02X): %s", frame_type,
               format_hex_pretty(&data.front(), data.size()).c_str());
  }

  // Send next command after each received frame
  if (this->next_command_ < SEPLOS_COMMAND_QUEUE_SIZE) {
    this->send_command_(SEPLOS_COMMAND_QUEUE[this->next_command_++ % SEPLOS_COMMAND_QUEUE_SIZE]);
  }
}

void SeplosBmsBle::decode_software_version_data_(const std::vector<uint8_t> &data) {
  ESP_LOGI(TAG, "Software version frame received");
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Byte Len Payload      Description                      Unit  Precision
  //  0    1  0x55         Start of frame
  //  1    1  0x14         Frame type (Response)
  //  2    1  0x81         Address

  //  3    16 0x30 0x2e 0x31 0x2e 0x31 0x30 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00    "0.1.10"
  this->publish_state_(this->software_version_text_sensor_, std::string(data.begin() + 3, data.end() - 1));

  //  19   1  0xaa         End of frame
}

void SeplosBmsBle::decode_hardware_version_data_(const std::vector<uint8_t> &data) {
  ESP_LOGI(TAG, "Hardware version frame received");
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Byte Len Payload      Description                      Unit  Precision
  //  0    1  0x55         Start of frame
  //  1    1  0x14         Frame type (Response)
  //  2    1  0x82         Address

  //  3    16 0x54 0x50 0x2d 0x4c 0x54 0x35 0x35 0x00 0x54 0x42 0x00 0x00 0x00 0x00 0x00 0x00    "TP-LT55" "TB"
  this->publish_state_(this->device_model_text_sensor_, std::string(data.begin() + 3, data.end() - 1));

  //  19   1  0xaa         End of frame
}

void SeplosBmsBle::decode_status_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGI(TAG, "Status frame received");
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Byte Len Payload      Description                      Unit  Precision
  //  0    1  0x55         Start of frame
  //  1    1  0x14         Frame type (Response)
  //  2    1  0x83         Address
  //  3    2  0x00 0x3c    State of charge (60%)              %   1.0    60%
  this->publish_state_(this->state_of_charge_sensor_, seplos_get_16bit(3) * 1.0f);

  //  5    2  0x14 0x72    Total voltage                      V   0.01f  52.34V
  float total_voltage = seplos_get_16bit(5) * 0.01f;
  this->publish_state_(this->total_voltage_sensor_, total_voltage);

  //  7    2  0x01 0x18    Average temperature               °C   0.1f   28.0°C
  this->publish_state_(this->average_temperature_sensor_, seplos_get_16bit(7) * 0.1f);

  //  9    2  0x00 0xe6    Ambient temperature               °C   0.1f   23.0°C
  this->publish_state_(this->ambient_temperature_sensor_, seplos_get_16bit(9) * 0.1f);

  //  11   2  0x00 0xf0    Mosfet temperature                °C   0.1f   24.0°C
  this->publish_state_(this->mosfet_temperature_sensor_, seplos_get_16bit(11) * 0.1f);

  //  13   2  0x00 0x00    Current (signed)
  float current = ((int16_t) seplos_get_16bit(13)) * 1.0f;
  this->publish_state_(this->current_sensor_, current);
  float power = total_voltage * current;
  this->publish_state_(this->power_sensor_, power);
  this->publish_state_(this->charging_power_sensor_, std::max(0.0f, power));               // 500W vs 0W -> 500W
  this->publish_state_(this->discharging_power_sensor_, std::abs(std::min(0.0f, power)));  // -500W vs 0W -> 500W

  //  15   2  0x30 0x30    Unknown (signed)

  //  17   2  0x00 0x64    State of health                    %   1.0    100%
  this->publish_state_(this->state_of_health_sensor_, seplos_get_16bit(17) * 1.0f);

  //  19   1  0xaa         End of frame
}

void SeplosBmsBle::decode_general_info_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGI(TAG, "General info frame received");
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Byte Len Payload      Description                      Unit  Precision
  //  0    1  0x55         Start of frame
  //  1    1  0x14         Frame type (Response)
  //  2    1  0x84         Address
  //  3    1  0x10         Cell count                             1.0    16
  //  4    1  0x04         Temp sensors                           1.0    16

  //  5    2  0x59 0xd8    Nominal capaciy                   Ah   0.01f  230.00 Ah
  this->publish_state_(this->nominal_capacity_sensor_, seplos_get_16bit(5) * 0.01f);

  //  7    2  0x36 0x48    Capacity remaining                Ah   0.01f  138.96 Ah
  this->publish_state_(this->capacity_remaining_sensor_, seplos_get_16bit(7) * 0.01f);

  //  9    2  0x00 0x00    Cycle count                            1.0    0
  this->publish_state_(this->charging_cycles_sensor_, seplos_get_16bit(9) * 1.0f);

  //  11   2  0x00 0x00    Voltage protection bitmask
  this->publish_state_(this->voltage_protection_bitmask_sensor_, seplos_get_16bit(11) * 1.0f);
  this->publish_state_(
      this->voltage_protection_text_sensor_,
      bitmask_to_string_(VOLTAGE_PROTECTION_ERRORS, VOLTAGE_PROTECTION_ERRORS_SIZE, seplos_get_16bit(11)));

  //  13   2  0x00 0x00    Current protection bitmask
  this->publish_state_(this->current_protection_bitmask_sensor_, seplos_get_16bit(13) * 1.0f);
  this->publish_state_(
      this->current_protection_text_sensor_,
      bitmask_to_string_(CURRENT_PROTECTION_ERRORS, CURRENT_PROTECTION_ERRORS_SIZE, seplos_get_16bit(13)));

  //  15   2  0x00 0x00    Temperature protection bitmask
  this->publish_state_(this->temperature_protection_bitmask_sensor_, seplos_get_16bit(15) * 1.0f);
  this->publish_state_(
      this->temperature_protection_text_sensor_,
      bitmask_to_string_(TEMPERATURE_PROTECTION_ERRORS, TEMPERATURE_PROTECTION_ERRORS_SIZE, seplos_get_16bit(15)));

  //  17   2  0x00 0x00    Error bitmask
  this->publish_state_(this->error_bitmask_sensor_, seplos_get_16bit(17) * 1.0f);
  this->publish_state_(this->errors_text_sensor_, bitmask_to_string_(ERRORS, ERRORS_SIZE, seplos_get_16bit(17)));

  //  19   1  0xaa         End of frame
}

void SeplosBmsBle::decode_mosfet_status_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGI(TAG, "Mosfet status frame received");
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Byte Len Payload      Description                      Unit  Precision
  //  0    1  0x55         Start of frame
  //  1    1  0x14         Frame type (Response)
  //  2    1  0x85         Address
  //  3    2  0x08 0x23    Mosfet Status Bitmask (0b100000100011)
  this->publish_state_(this->charging_binary_sensor_, (bool) check_bit_(seplos_get_16bit(3), 1));
  this->publish_state_(this->discharging_binary_sensor_, (bool) check_bit_(seplos_get_16bit(3), 2));
  this->publish_state_(this->limiting_current_binary_sensor_,
                       check_bit_(seplos_get_16bit(3), 16) || check_bit_(seplos_get_16bit(3), 32));

  //  5    2  0x00 0x00    Overvoltage protection bitmask
  //  7    2  0x00 0x00    Undervoltage protection bitmask
  //  9    2  0x00 0x00    High Alarm Bitmask
  //  11   2  0x00 0x00    Low Alarm Bitmask
  //  13   2  0x00 0x00    Balancing? Bitmask
  //  15   2  0x00 0x00    Unused
  //  17   2  0x00 0x00    Unused
  //  19   1  0xaa         End of frame
}

void SeplosBmsBle::decode_temperature_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGI(TAG, "Temperature frame received");
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Byte Len Payload      Description                      Unit  Precision
  //  0    1  0x55         Start of frame
  //  1    1  0x14         Frame type (Response)
  //  2    1  0x87         Address
  //  3    2  0x00 0xe6    Temperature 1                    °C    0.1f  23.0°C
  //  5    2  0x00 0xe6    Temperature 2                    °C    0.1f  23.0°C
  //  7    2  0x00 0xe6    Temperature 3                    °C    0.1f  23.0°C
  //  9    2  0x00 0xe6    Temperature 4                    °C    0.1f  23.0°C
  //  11   2  0x00 0x00    Temperature 5
  //  13   2  0x00 0x00    Temperature 6
  //  15   2  0x00 0x00    Temperature 7
  //  17   2  0x00 0x00    Temperature 8
  for (uint8_t i = 0; i < 8; i++) {
    this->publish_state_(this->temperatures_[i].temperature_sensor_, ((int16_t) seplos_get_16bit((i * 2) + 3)) * 0.1f);
  }

  //  19   1  0xaa         End of frame
}

void SeplosBmsBle::decode_cell_voltages_data_(const uint8_t &chunk, const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  uint8_t offset = 8 * chunk;

  ESP_LOGI(TAG, "Cell voltages frame (chunk %d) received", chunk);
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Byte Len Payload      Description                      Unit  Precision
  //  0    1  0x55         Start of frame
  //  1    1  0x14         Frame type (Response)
  //  2    1  0x87         Address
  //  3    2  0x96 0x0C    Cell voltage 1                   V     0.001f  3.265V
  //  5    2  0x97 0x0C    Cell voltage 2                   V     0.001f  3.285V
  //  7    2  0x98 0x0C    Cell voltage 3                   V     0.001f  3.282V
  //  9    2  0x98 0x0C    Cell voltage 4                   V     0.001f
  //  11   2  0x96 0x0C    Cell voltage 5                   V     0.001f
  //  13   2  0x96 0x0C    Cell voltage 6                   V     0.001f
  //  15   2  0x98 0x0C    Cell voltage 7                   V     0.001f
  //  17   2  0x98 0x0C    Cell voltage 8                   V     0.001f
  for (uint8_t i = 0; i < 8; i++) {
    float cell_voltage = seplos_get_16bit((i * 2) + 3) * 0.001f;
    if (cell_voltage > 0 && cell_voltage < this->min_cell_voltage_) {
      this->min_cell_voltage_ = cell_voltage;
      this->min_voltage_cell_ = i + offset + 1;
    }
    if (cell_voltage > this->max_cell_voltage_) {
      this->max_cell_voltage_ = cell_voltage;
      this->max_voltage_cell_ = i + offset + 1;
    }
    this->publish_state_(this->cells_[i + offset].cell_voltage_sensor_, cell_voltage);
  }

  // Publish aggregated sensors at the last chunk. Must be improved if 3 chunks are retrieved.
  if (chunk == 1) {
    this->publish_state_(this->min_cell_voltage_sensor_, this->min_cell_voltage_);
    this->publish_state_(this->max_cell_voltage_sensor_, this->max_cell_voltage_);
    this->publish_state_(this->max_voltage_cell_sensor_, (float) this->max_voltage_cell_);
    this->publish_state_(this->min_voltage_cell_sensor_, (float) this->min_voltage_cell_);
    this->publish_state_(this->delta_cell_voltage_sensor_, this->max_cell_voltage_ - this->min_cell_voltage_);
  }

  //  19   1  0xaa         End of frame
}

void SeplosBmsBle::dump_config() {  // NOLINT(google-readability-function-size,readability-function-size)
  ESP_LOGCONFIG(TAG, "SeplosBmsBle:");

  LOG_BINARY_SENSOR("", "Charging", this->charging_binary_sensor_);
  LOG_BINARY_SENSOR("", "Discharging", this->discharging_binary_sensor_);
  LOG_BINARY_SENSOR("", "Limiting current", this->limiting_current_binary_sensor_);

  LOG_SENSOR("", "Total voltage", this->total_voltage_sensor_);
  LOG_SENSOR("", "Current", this->current_sensor_);
  LOG_SENSOR("", "Power", this->power_sensor_);
  LOG_SENSOR("", "Charging power", this->charging_power_sensor_);
  LOG_SENSOR("", "Discharging power", this->discharging_power_sensor_);
  LOG_SENSOR("", "Capacity remaining", this->capacity_remaining_sensor_);
  LOG_SENSOR("", "Voltage protection bitmask", this->voltage_protection_bitmask_sensor_);
  LOG_SENSOR("", "Current protection bitmask", this->current_protection_bitmask_sensor_);
  LOG_SENSOR("", "Temperature protection bitmask", this->temperature_protection_bitmask_sensor_);
  LOG_SENSOR("", "Error bitmask", this->error_bitmask_sensor_);
  LOG_SENSOR("", "State of charge", this->state_of_charge_sensor_);
  LOG_SENSOR("", "Nominal capacity", this->nominal_capacity_sensor_);
  LOG_SENSOR("", "Charging cycles", this->charging_cycles_sensor_);
  LOG_SENSOR("", "Min cell voltage", this->min_cell_voltage_sensor_);
  LOG_SENSOR("", "Max cell voltage", this->max_cell_voltage_sensor_);
  LOG_SENSOR("", "Min voltage cell", this->min_voltage_cell_sensor_);
  LOG_SENSOR("", "Max voltage cell", this->max_voltage_cell_sensor_);
  LOG_SENSOR("", "Delta cell voltage", this->delta_cell_voltage_sensor_);
  LOG_SENSOR("", "Average temperature", this->average_temperature_sensor_);
  LOG_SENSOR("", "Ambient temperature", this->ambient_temperature_sensor_);
  LOG_SENSOR("", "Mosfet temperature", this->mosfet_temperature_sensor_);
  LOG_SENSOR("", "State of health", this->state_of_charge_sensor_);
  LOG_SENSOR("", "Temperature 1", this->temperatures_[0].temperature_sensor_);
  LOG_SENSOR("", "Temperature 2", this->temperatures_[1].temperature_sensor_);
  LOG_SENSOR("", "Temperature 3", this->temperatures_[2].temperature_sensor_);
  LOG_SENSOR("", "Temperature 4", this->temperatures_[3].temperature_sensor_);
  LOG_SENSOR("", "Temperature 5", this->temperatures_[4].temperature_sensor_);
  LOG_SENSOR("", "Temperature 6", this->temperatures_[5].temperature_sensor_);
  LOG_SENSOR("", "Temperature 7", this->temperatures_[6].temperature_sensor_);
  LOG_SENSOR("", "Temperature 8", this->temperatures_[7].temperature_sensor_);
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
  LOG_SENSOR("", "Cell Voltage 17", this->cells_[16].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 18", this->cells_[17].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 19", this->cells_[18].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 20", this->cells_[19].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 21", this->cells_[20].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 22", this->cells_[21].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 23", this->cells_[22].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 24", this->cells_[23].cell_voltage_sensor_);

  LOG_TEXT_SENSOR("", "Voltage protection", this->voltage_protection_text_sensor_);
  LOG_TEXT_SENSOR("", "Current protection", this->current_protection_text_sensor_);
  LOG_TEXT_SENSOR("", "Temperature protection", this->temperature_protection_text_sensor_);
  LOG_TEXT_SENSOR("", "Errors", this->errors_text_sensor_);
}

void SeplosBmsBle::publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state) {
  if (binary_sensor == nullptr)
    return;

  binary_sensor->publish_state(state);
}

void SeplosBmsBle::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor == nullptr)
    return;

  sensor->publish_state(value);
}

void SeplosBmsBle::publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state) {
  if (text_sensor == nullptr)
    return;

  text_sensor->publish_state(state);
}

void SeplosBmsBle::write_register(uint8_t address, uint16_t value) {
  // this->send_command_(SEPLOS_CMD_WRITE, SEPLOS_CMD_MOS);  // @TODO: Pass value
}

bool SeplosBmsBle::send_command_(uint8_t function) {
  uint8_t frame[4];

  frame[0] = SEPLOS_PKT_START;
  frame[1] = SEPLOS_FUNCTION_REQUEST;
  frame[2] = function;
  frame[3] = SEPLOS_PKT_END;

  ESP_LOGD(TAG, "Send command (handle 0x%02X): %s", this->char_command_handle_,
           format_hex_pretty(frame, sizeof(frame)).c_str());

  auto status =
      esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), this->char_command_handle_,
                               sizeof(frame), frame, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);

  if (status) {
    ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status=%d", this->parent_->address_str().c_str(), status);
  }

  return (status == 0);
}

std::string SeplosBmsBle::bitmask_to_string_(const char *const messages[], const uint8_t &messages_size,
                                             const uint16_t &mask) {
  std::string values = "";
  if (mask) {
    for (int i = 0; i < messages_size; i++) {
      if (mask & (1 << i)) {
        values.append(messages[i]);
        values.append(";");
      }
    }
    if (!values.empty()) {
      values.pop_back();
    }
  }
  return values;
}

}  // namespace seplos_bms_ble
}  // namespace esphome
