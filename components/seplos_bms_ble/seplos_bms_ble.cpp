#include "seplos_bms_ble.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace seplos_bms_ble {

static uint16_t crc_xmodem(const uint8_t *data, uint16_t len) {
  uint16_t crc = 0x0000;
  for (uint16_t i = 0; i < len; i++) {
    crc ^= (uint16_t) data[i] << 8;
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

static const char *const TAG = "seplos_bms_ble";

static const char *const ALARM_EVENT1_MESSAGES[8] = {
    "Voltage sensing failure",      // Bit 0
    "Temperature sensing failure",  // Bit 1
    "Current sensing failure",      // Bit 2
    "Key switch failure",           // Bit 3
    "Cell voltage diff failure",    // Bit 4
    "Charging switch failure",      // Bit 5
    "Discharge switch failure",     // Bit 6
    "Current limit switch failure"  // Bit 7
};

static const char *const ALARM_EVENT2_MESSAGES[8] = {
    "Single high voltage alarm",       // Bit 0
    "Single overvoltage protection",   // Bit 1
    "Single low voltage alarm",        // Bit 2
    "Single undervoltage protection",  // Bit 3
    "Total high voltage alarm",        // Bit 4
    "Total overvoltage protection",    // Bit 5
    "Total low voltage alarm",         // Bit 6
    "Total undervoltage protection"    // Bit 7
};

static const char *const ALARM_EVENT3_MESSAGES[8] = {
    "Charging high temp alarm",       // Bit 0
    "Charging overtemp protection",   // Bit 1
    "Charging low temp alarm",        // Bit 2
    "Charging undertemp protection",  // Bit 3
    "Discharge high temp alarm",      // Bit 4
    "Discharge overtemp protection",  // Bit 5
    "Discharge low temp alarm",       // Bit 6
    "Discharge undertemp protection"  // Bit 7
};

static const char *const ALARM_EVENT4_MESSAGES[8] = {
    "Ambient high temp alarm",       // Bit 0
    "Ambient overtemp protection",   // Bit 1
    "Ambient low temp alarm",        // Bit 2
    "Ambient undertemp protection",  // Bit 3
    "Power overtemp protection",     // Bit 4
    "Power high temp alarm",         // Bit 5
    "Battery low temp heating",      // Bit 6
    "Secondary trip protection"      // Bit 7
};

static const char *const ALARM_EVENT5_MESSAGES[8] = {
    "Charging overcurrent alarm",        // Bit 0
    "Charging overcurrent protection",   // Bit 1
    "Discharge overcurrent alarm",       // Bit 2
    "Discharge overcurrent protection",  // Bit 3
    "Transient overcurrent protection",  // Bit 4
    "Output short circuit protection",   // Bit 5
    "Transient overcurrent lockout",     // Bit 6
    "Output short circuit lockout"       // Bit 7
};

static const char *const ALARM_EVENT6_MESSAGES[8] = {
    "Charging high voltage protection",    // Bit 0
    "Intermittent power replenishment",    // Bit 1
    "Remaining capacity alarm",            // Bit 2
    "Remaining capacity protection",       // Bit 3
    "Low voltage charging prohibited",     // Bit 4
    "Output reverse polarity protection",  // Bit 5
    "Output connection failure",           // Bit 6
    "Internal alarm"                       // Bit 7
};

static const char *const ALARM_EVENT7_MESSAGES[8] = {
    "Internal alarm 1",            // Bit 0
    "Internal alarm 2",            // Bit 1
    "Internal alarm 3",            // Bit 2
    "Internal alarm 4",            // Bit 3
    "Automatic charging waiting",  // Bit 4
    "Manual charging waiting",     // Bit 5
    "Internal alarm 6",            // Bit 6
    "Internal alarm 7"             // Bit 7
};

static const char *const ALARM_EVENT8_MESSAGES[8] = {
    "EEP storage failure",              // Bit 0
    "RTC clock failure",                // Bit 1
    "Voltage calibration not done",     // Bit 2
    "Current calibration not done",     // Bit 3
    "Zero point calibration not done",  // Bit 4
    "Calendar not synchronized",        // Bit 5
    "Internal system error 6",          // Bit 6
    "Internal system error 7"           // Bit 7
};

static const uint16_t SEPLOS_BMS_SERVICE_UUID = 0xFF00;
static const uint16_t SEPLOS_BMS_NOTIFY_CHARACTERISTIC_UUID = 0xFF01;   // handle 0x12
static const uint16_t SEPLOS_BMS_CONTROL_CHARACTERISTIC_UUID = 0xFF02;  // handle 0x14

static const uint16_t MAX_RESPONSE_SIZE = 200;

static const uint16_t SEPLOS_PKT_START = 0x7E;
static const uint16_t SEPLOS_PKT_END = 0x0D;

static const uint8_t SEPLOS_CMD_GET_SETTINGS = 0x47;
static const uint8_t SEPLOS_CMD_GET_MANUFACTURER_INFO = 0x51;
static const uint8_t SEPLOS_CMD_GET_SINGLE_MACHINE_DATA = 0x61;
static const uint8_t SEPLOS_CMD_GET_PARALLEL_DATA = 0x62;
static const uint8_t SEPLOS_CMD_PROTOCOL_SWITCH_CAN = 0x63;
static const uint8_t SEPLOS_CMD_PROTOCOL_SWITCH_RS485 = 0x64;
static const uint8_t SEPLOS_CMD_SET_BMS_PARAMETER = 0xA1;
static const uint8_t SEPLOS_CMD_SET_DEVICE_GROUP_NUMBER_NAME = 0x65;
static const uint8_t SEPLOS_CMD_SET_MOSFET_CONTROL = 0xAA;

struct SeplosCommand {
  uint8_t function;
  std::vector<uint8_t> payload;
};

static const uint8_t SEPLOS_COMMAND_QUEUE_SIZE = 4;
static const SeplosCommand SEPLOS_COMMAND_QUEUE[SEPLOS_COMMAND_QUEUE_SIZE] = {
    {SEPLOS_CMD_GET_SETTINGS, {0x00}},
    {SEPLOS_CMD_GET_MANUFACTURER_INFO, {}},
    {SEPLOS_CMD_GET_SINGLE_MACHINE_DATA, {0x00}},
    {SEPLOS_CMD_GET_PARALLEL_DATA, {}}};

void SeplosBmsBle::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                       esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      this->node_state = espbt::ClientState::IDLE;
      this->publish_state_(this->online_status_binary_sensor_, false);

      // Clear frame assembly state on disconnect
      this->frame_buffer_.clear();
      this->next_command_ = 0;
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
        ESP_LOGE(TAG, "[%s] No notify service found at device, not an Seplos v2 BMS..?",
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
      this->publish_state_(this->online_status_binary_sensor_, true);
      this->send_command(SEPLOS_COMMAND_QUEUE[0].function, SEPLOS_COMMAND_QUEUE[0].payload);
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

void SeplosBmsBle::update() {
  if (this->node_state != espbt::ClientState::ESTABLISHED) {
    ESP_LOGW(TAG, "[%s] Not connected", this->parent_->address_str().c_str());
    return;
  }

  // Loop through all commands if connected
  if (this->next_command_ != 0) {
    ESP_LOGW(TAG,
             "Command queue (%d of %d) was not completely processed. "
             "Please increase the update_interval if you see this warning frequently",
             this->next_command_, SEPLOS_COMMAND_QUEUE_SIZE);
  }
  this->next_command_ = 0;
  this->send_command(SEPLOS_COMMAND_QUEUE[this->next_command_].function,
                     SEPLOS_COMMAND_QUEUE[this->next_command_].payload);
  this->next_command_++;
}

void SeplosBmsBle::assemble(const uint8_t *data, uint16_t length) {
  if (this->frame_buffer_.size() > MAX_RESPONSE_SIZE) {
    ESP_LOGW(TAG, "Frame dropped because of invalid length");
    this->frame_buffer_.clear();
  }

  // Flush buffer on every preamble (start of frame)
  if (length >= 1 && data[0] == SEPLOS_PKT_START) {
    this->frame_buffer_.clear();
  }

  this->frame_buffer_.insert(this->frame_buffer_.end(), data, data + length);

  if (this->frame_buffer_.size() >= 7) {
    const uint8_t *raw = &this->frame_buffer_[0];

    uint16_t data_len = (uint16_t(raw[5]) << 8) | uint16_t(raw[6]);
    size_t frame_len = 7 + data_len + 2 + 1;  // header + payload + CRC + EOF

    if (frame_len > MAX_RESPONSE_SIZE) {
      ESP_LOGW(TAG, "Frame too large: %zu bytes", frame_len);
      this->frame_buffer_.clear();
      return;
    }

    // Check if we have received the expected complete frame
    if (this->frame_buffer_.size() >= frame_len) {
      // Verify frame ends with SEPLOS_PKT_END at expected position
      if (raw[frame_len - 1] == SEPLOS_PKT_END) {
        // Validate CRC (last 2 bytes before end marker)
        uint16_t computed_crc = crc_xmodem(raw + 1, frame_len - 4);  // Exclude start, CRC, and end
        uint16_t remote_crc = (uint16_t(raw[frame_len - 3]) << 8) | uint16_t(raw[frame_len - 2]);

        if (computed_crc != remote_crc) {
          ESP_LOGW(TAG, "CRC check failed! 0x%04X != 0x%04X", computed_crc, remote_crc);
          this->frame_buffer_.clear();
          return;
        }

        std::vector<uint8_t> data(raw, raw + frame_len);
        this->decode(data);
        this->frame_buffer_.clear();
      } else {
        ESP_LOGW(TAG, "Frame end marker missing at expected position %zu", frame_len - 1);
        this->frame_buffer_.clear();
      }
    }
  }
}

void SeplosBmsBle::decode(const std::vector<uint8_t> &data) {
  uint8_t function = data[3];

  switch (function) {
    case SEPLOS_CMD_GET_SINGLE_MACHINE_DATA:
      this->decode_single_machine_data_(data);
      break;
    case SEPLOS_CMD_GET_MANUFACTURER_INFO:
      this->decode_manufacturer_info_data_(data);
      break;
    case SEPLOS_CMD_GET_SETTINGS:
      this->decode_settings_data_(data);
      break;
    case SEPLOS_CMD_GET_PARALLEL_DATA:
      this->decode_parallel_data_(data);
      break;
    case SEPLOS_CMD_PROTOCOL_SWITCH_CAN:
      ESP_LOGI(TAG, "Protocol switch CAN frame (%zu bytes) received", data.size());
      ESP_LOGD(TAG, "  Decoding not implemented yet");
      break;
    case SEPLOS_CMD_PROTOCOL_SWITCH_RS485:
      ESP_LOGI(TAG, "Protocol switch RS485 frame (%zu bytes) received", data.size());
      ESP_LOGD(TAG, "  Decoding not implemented yet");
      break;
    case SEPLOS_CMD_SET_BMS_PARAMETER:
      ESP_LOGI(TAG, "BMS parameter frame (%zu bytes) received", data.size());
      ESP_LOGD(TAG, "  Decoding not implemented yet");
      break;
    case SEPLOS_CMD_SET_DEVICE_GROUP_NUMBER_NAME:
      ESP_LOGI(TAG, "Device group number name frame (%zu bytes) received", data.size());
      ESP_LOGD(TAG, "  Decoding not implemented yet");
      break;
    case SEPLOS_CMD_SET_MOSFET_CONTROL:
      ESP_LOGI(TAG, "Switch control response (%zu bytes) received", data.size());
      ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());
      if (data.size() >= 9) {
        uint8_t result = data[7];
        ESP_LOGI(TAG, "Switch control result: %s", result == 0x00 ? "SUCCESS" : "FAILED");
      }
      break;
    default:
      ESP_LOGW(TAG, "Unhandled response received (function 0x%02X): %s", function,
               format_hex_pretty(&data.front(), data.size()).c_str());
  }

  // Send next command after each received frame
  if (this->next_command_ < SEPLOS_COMMAND_QUEUE_SIZE) {
    this->send_command(SEPLOS_COMMAND_QUEUE[this->next_command_].function,
                       SEPLOS_COMMAND_QUEUE[this->next_command_].payload);
    this->next_command_++;
  }
}

void SeplosBmsBle::decode_manufacturer_info_data_(const std::vector<uint8_t> &data) {
  ESP_LOGI(TAG, "Hardware version frame (%zu bytes) received", data.size());
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Expected frame size: 7 (header) + 35 (data) + 2 (CRC) + 1 (EOF) = 45 bytes
  if (data.size() < 45) {
    ESP_LOGW(TAG, "Hardware version frame too short (%d bytes, expected 45)", data.size());
    return;
  }

  std::string device_model(data.begin() + 7, data.begin() + 27);
  device_model.erase(device_model.find_last_not_of(' ') + 1);
  this->publish_state_(this->device_model_text_sensor_, device_model);

  std::string hardware_version(data.begin() + 27, data.begin() + 37);
  hardware_version.erase(hardware_version.find_last_not_of(' ') + 1);
  this->publish_state_(this->hardware_version_text_sensor_, hardware_version);

  this->publish_state_(this->software_version_text_sensor_, std::to_string(data[37]) + "." + std::to_string(data[38]));

  ESP_LOGI(TAG, "CAN Protocol: %s", this->interpret_can_protocol(data[39]).c_str());

  ESP_LOGI(TAG, "RS485 Protocol: %s", this->interpret_rs485_protocol(data[40]).c_str());

  ESP_LOGI(TAG, "Battery Type: %s", this->interpret_battery_type(data[41]).c_str());

  ESP_LOGI(TAG, "Number of Secondaries: %d", data[42]);
}

std::string SeplosBmsBle::interpret_can_protocol(uint8_t value) {
  switch (value) {
    case 0x00:
      return "Unset";
    case 0x01:
      return "Pylontech (PN-GDLT)";
    case 0x02:
      return "Growatt (GRWT)";
    case 0x03:
      return "Victron (VCTR)";
    case 0x04:
      return "SMA (SMA-SOFAR)";
    case 0x05:
      return "GINL (SMA-OffGrid?)";
    case 0x06:
      return "Studer (STUD)";
    default: {
      char hex_str[10];
      snprintf(hex_str, sizeof(hex_str), "0x%02X", value);
      return "Unknown (" + std::string(hex_str) + ")";
    }
  }
}

std::string SeplosBmsBle::interpret_rs485_protocol(uint8_t value) {
  switch (value) {
    case 0x00:
      return "Unset";
    case 0x01:
      return "Pylontech (PN)";
    case 0x02:
      return "Growatt (GRWT)";
    case 0x03:
      return "Voltronic (VLTC)";
    case 0x04:
      return "Sofar (SF)";
    case 0x05:
      return "Luxpowertek (LUXP)";
    case 0x06:
      return "Studer (STUD)";
    default: {
      char hex_str[10];
      snprintf(hex_str, sizeof(hex_str), "0x%02X", value);
      return "Unknown (" + std::string(hex_str) + ")";
    }
  }
}

std::string SeplosBmsBle::interpret_battery_type(uint8_t value) {
  switch (value) {
    case 0x46:
      return "LFP";
    case 0x47:
      return "NCM";
    case 0x48:
      return "LCO";
    case 0x49:
      return "LTO";
    case 0x4A:
      return "Reserved";
    default: {
      char hex_str[10];
      snprintf(hex_str, sizeof(hex_str), "0x%02X", value);
      return "Unknown (" + std::string(hex_str) + ")";
    }
  }
}

void SeplosBmsBle::decode_settings_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t { return (uint16_t(data[i]) << 8) | uint16_t(data[i + 1]); };

  ESP_LOGI(TAG, "Settings frame (%zu bytes) received", data.size());
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  if (data.size() < 145) {
    ESP_LOGW(TAG, "Settings frame too short (%d bytes)", data.size());
    return;
  }

  // uint16_t data_len = seplos_get_16bit(5);
  ESP_LOGD(TAG, "Group number: %d", data[7]);
  ESP_LOGD(TAG, "Parameter count: %d", data[8]);

  // Voltage parameters
  ESP_LOGD(TAG, "Single high voltage alarm: %.3f V", seplos_get_16bit(9) * 0.001f);
  ESP_LOGD(TAG, "Single high pressure recovery: %.3f V", seplos_get_16bit(11) * 0.001f);
  ESP_LOGD(TAG, "Single unit low voltage alarm: %.3f V", seplos_get_16bit(13) * 0.001f);
  ESP_LOGD(TAG, "Single unit low pressure recovery: %.3f V", seplos_get_16bit(15) * 0.001f);
  ESP_LOGD(TAG, "Single unit overvoltage protection: %.3f V", seplos_get_16bit(17) * 0.001f);
  ESP_LOGD(TAG, "Cell overvoltage recovery: %.3f V", seplos_get_16bit(19) * 0.001f);
  ESP_LOGD(TAG, "Single unit under voltage protection: %.3f V", seplos_get_16bit(21) * 0.001f);
  ESP_LOGD(TAG, "Single unit undervoltage recovery: %.3f V", seplos_get_16bit(23) * 0.001f);
  ESP_LOGD(TAG, "Balanced turn-on voltage: %.3f V", seplos_get_16bit(25) * 0.001f);
  ESP_LOGD(TAG, "Battery cell low voltage charging is prohibited: %.3f V", seplos_get_16bit(27) * 0.001f);

  // Total voltage parameters
  ESP_LOGD(TAG, "Total pressure high pressure alarm: %.2f V", seplos_get_16bit(29) * 0.01f);
  ESP_LOGD(TAG, "Total pressure high pressure recovery: %.2f V", seplos_get_16bit(31) * 0.01f);
  ESP_LOGD(TAG, "Low total pressure alarm: %.2f V", seplos_get_16bit(33) * 0.01f);
  ESP_LOGD(TAG, "Total pressure low pressure recovery: %.2f V", seplos_get_16bit(35) * 0.01f);
  ESP_LOGD(TAG, "Total voltage overvoltage protection: %.2f V", seplos_get_16bit(37) * 0.01f);
  ESP_LOGD(TAG, "Total pressure overvoltage recovery: %.2f V", seplos_get_16bit(39) * 0.01f);
  ESP_LOGD(TAG, "Total voltage undervoltage protection: %.2f V", seplos_get_16bit(41) * 0.01f);
  ESP_LOGD(TAG, "Total voltage and undervoltage recovery: %.2f V", seplos_get_16bit(43) * 0.01f);
  ESP_LOGD(TAG, "Charging overvoltage protection: %.2f V", seplos_get_16bit(45) * 0.01f);
  ESP_LOGD(TAG, "Charging overvoltage recovery: %.2f V", seplos_get_16bit(47) * 0.01f);

  // Temperature parameters
  ESP_LOGD(TAG, "Charging high temperature alarm: %.1f °C", seplos_get_16bit(49) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Charging high temperature recovery: %.1f °C", seplos_get_16bit(51) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Charging low temperature alarm: %.1f °C", seplos_get_16bit(53) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Charging low temperature recovery: %.1f °C", seplos_get_16bit(55) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Charging over-temperature protection: %.1f °C", seplos_get_16bit(57) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Charging over-temperature recovery: %.1f °C", seplos_get_16bit(59) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Charging under-temperature protection: %.1f °C", seplos_get_16bit(61) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Charging under-temperature recovery: %.1f °C", seplos_get_16bit(63) * 0.1f - 273.15f);

  ESP_LOGD(TAG, "Discharge high temperature alarm: %.1f °C", seplos_get_16bit(65) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Discharge high temperature recovery: %.1f °C", seplos_get_16bit(67) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Discharge low temperature alarm: %.1f °C", seplos_get_16bit(69) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Discharge low temperature recovery: %.1f °C", seplos_get_16bit(71) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Discharge over temperature protection: %.1f °C", seplos_get_16bit(73) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Discharge over-temperature recovery: %.1f °C", seplos_get_16bit(75) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Discharge under-temperature protection: %.1f °C", seplos_get_16bit(77) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Discharge under-temperature recovery: %.1f °C", seplos_get_16bit(79) * 0.1f - 273.15f);

  ESP_LOGD(TAG, "Battery core low temperature heating: %.1f °C", seplos_get_16bit(81) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Battery cell low temperature recovery: %.1f °C", seplos_get_16bit(83) * 0.1f - 273.15f);

  ESP_LOGD(TAG, "Environmental high temperature alarm: %.1f °C", seplos_get_16bit(85) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Environmental high temperature recovery: %.1f °C", seplos_get_16bit(87) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Environmental low temperature alarm: %.1f °C", seplos_get_16bit(89) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Ambient low temperature recovery: %.1f °C", seplos_get_16bit(91) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Environmental over-temperature protection: %.1f °C", seplos_get_16bit(93) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Environment over-temperature recovery: %.1f °C", seplos_get_16bit(95) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Environmental under-temperature protection: %.1f °C", seplos_get_16bit(97) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Environmental low temperature recovery: %.1f °C", seplos_get_16bit(99) * 0.1f - 273.15f);

  ESP_LOGD(TAG, "Power high temperature alarm: %.1f °C", seplos_get_16bit(101) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Power high temperature recovery: %.1f °C", seplos_get_16bit(103) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Power over temperature protection: %.1f °C", seplos_get_16bit(105) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Power over temperature recovery: %.1f °C", seplos_get_16bit(107) * 0.1f - 273.15f);

  // Current parameters (0.01A)
  ESP_LOGD(TAG, "Charging overcurrent alarm: %.2f A", seplos_get_16bit(109) * 0.01f);
  ESP_LOGD(TAG, "Charging overcurrent recovery: %.2f A", seplos_get_16bit(111) * 0.01f);
  ESP_LOGD(TAG, "Discharge overcurrent alarm: %.2f A", seplos_get_16bit(113) * 0.01f);
  ESP_LOGD(TAG, "Discharge overcurrent recovery: %.2f A", seplos_get_16bit(115) * 0.01f);
  ESP_LOGD(TAG, "Charging overcurrent protection: %.2f A", seplos_get_16bit(117) * 0.01f);
  ESP_LOGD(TAG, "Discharge overcurrent protection: %.2f A", seplos_get_16bit(119) * 0.01f);
  ESP_LOGD(TAG, "Transient overcurrent protection: %.2f A", seplos_get_16bit(121) * 0.01f);

  // Timing and capacity
  ESP_LOGD(TAG, "Output soft start delay: %d ms", seplos_get_16bit(123));
  ESP_LOGD(TAG, "Battery rated capacity: %.2f Ah", seplos_get_16bit(125) * 0.01f);
  ESP_LOGD(TAG, "Battery remaining capacity: %.2f Ah", seplos_get_16bit(127) * 0.01f);

  // 1-byte parameters
  ESP_LOGD(TAG, "Cell failure voltage difference: %.2f V", data[130] * 0.01f);
  ESP_LOGD(TAG, "Battery failure recovery: %.2f V", data[131] * 0.01f);
  ESP_LOGD(TAG, "Equilibrium opening pressure difference: %.3f V", data[132] * 0.001f);
  ESP_LOGD(TAG, "Equalization end pressure difference: %.3f V", data[133] * 0.001f);
  ESP_LOGD(TAG, "Static equilibrium time: %d h", data[134]);
  ESP_LOGD(TAG, "Number of battery cells in series: %d", data[135]);

  // Function switches with detailed bitmask interpretation
  uint8_t switch1 = data[136];
  ESP_LOGD(TAG, "Function switch 1: 0x%02X", switch1);
  if (switch1 & 0x01)
    ESP_LOGD(TAG, "  - Voltage sensing failure enabled");
  if (switch1 & 0x02)
    ESP_LOGD(TAG, "  - Temperature sensing failure enabled");
  if (switch1 & 0x04)
    ESP_LOGD(TAG, "  - Current sensing failure enabled");
  if (switch1 & 0x08)
    ESP_LOGD(TAG, "  - Key switch failure enabled");
  if (switch1 & 0x10)
    ESP_LOGD(TAG, "  - Cell voltage difference failure enabled");
  if (switch1 & 0x20)
    ESP_LOGD(TAG, "  - Charging switch failure enabled");
  if (switch1 & 0x40)
    ESP_LOGD(TAG, "  - Discharge switch failure enabled");
  if (switch1 & 0x80)
    ESP_LOGD(TAG, "  - Current limit switch failure enabled");

  uint8_t switch2 = data[137];
  ESP_LOGD(TAG, "Function switch 2: 0x%02X", switch2);
  if (switch2 & 0x01)
    ESP_LOGD(TAG, "  - Single high voltage alarm enabled");
  if (switch2 & 0x02)
    ESP_LOGD(TAG, "  - Single overvoltage protection enabled");
  if (switch2 & 0x04)
    ESP_LOGD(TAG, "  - Single unit low voltage alarm enabled");
  if (switch2 & 0x08)
    ESP_LOGD(TAG, "  - Single unit undervoltage protection enabled");
  if (switch2 & 0x10)
    ESP_LOGD(TAG, "  - Total pressure high voltage alarm enabled");
  if (switch2 & 0x20)
    ESP_LOGD(TAG, "  - Total voltage overvoltage protection enabled");
  if (switch2 & 0x40)
    ESP_LOGD(TAG, "  - Total pressure low pressure alarm enabled");
  if (switch2 & 0x80)
    ESP_LOGD(TAG, "  - Total voltage undervoltage protection enabled");

  uint8_t switch3 = data[138];
  ESP_LOGD(TAG, "Function switch 3: 0x%02X", switch3);
  if (switch3 & 0x01)
    ESP_LOGD(TAG, "  - Charging high temperature alarm enabled");
  if (switch3 & 0x02)
    ESP_LOGD(TAG, "  - Charging over-temperature protection enabled");
  if (switch3 & 0x04)
    ESP_LOGD(TAG, "  - Charging low temperature alarm enabled");
  if (switch3 & 0x08)
    ESP_LOGD(TAG, "  - Charging under-temperature protection enabled");
  if (switch3 & 0x10)
    ESP_LOGD(TAG, "  - Discharge high temperature alarm enabled");
  if (switch3 & 0x20)
    ESP_LOGD(TAG, "  - Discharge over-temperature protection enabled");
  if (switch3 & 0x40)
    ESP_LOGD(TAG, "  - Discharge low temperature alarm enabled");
  if (switch3 & 0x80)
    ESP_LOGD(TAG, "  - Discharge under-temperature protection enabled");

  uint8_t switch4 = data[139];
  ESP_LOGD(TAG, "Function switch 4: 0x%02X", switch4);
  if (switch4 & 0x01)
    ESP_LOGD(TAG, "  - Ambient high temperature alarm enabled");
  if (switch4 & 0x02)
    ESP_LOGD(TAG, "  - Environmental over-temperature protection enabled");
  if (switch4 & 0x04)
    ESP_LOGD(TAG, "  - Ambient low temperature alarm enabled");
  if (switch4 & 0x08)
    ESP_LOGD(TAG, "  - Environmental under-temperature protection enabled");
  if (switch4 & 0x10)
    ESP_LOGD(TAG, "  - Power over-temperature protection enabled");
  if (switch4 & 0x20)
    ESP_LOGD(TAG, "  - Power high temperature alarm enabled");
  if (switch4 & 0x40)
    ESP_LOGD(TAG, "  - Battery core low-temperature heating enabled");
  if (switch4 & 0x80)
    ESP_LOGD(TAG, "  - Secondary trip protection enabled");

  uint8_t switch5 = data[140];
  ESP_LOGD(TAG, "Function switch 5: 0x%02X", switch5);
  if (switch5 & 0x01)
    ESP_LOGD(TAG, "  - Charging overcurrent alarm enabled");
  if (switch5 & 0x02)
    ESP_LOGD(TAG, "  - Charging overcurrent protection enabled");
  if (switch5 & 0x04)
    ESP_LOGD(TAG, "  - Discharge overcurrent alarm enabled");
  if (switch5 & 0x08)
    ESP_LOGD(TAG, "  - Discharge overcurrent protection enabled");
  if (switch5 & 0x10)
    ESP_LOGD(TAG, "  - Transient overcurrent protection enabled");
  if (switch5 & 0x20)
    ESP_LOGD(TAG, "  - Output short circuit protection enabled");
  if (switch5 & 0x40)
    ESP_LOGD(TAG, "  - Transient overcurrent lockout enabled");
  if (switch5 & 0x80)
    ESP_LOGD(TAG, "  - Output short circuit lockout enabled");

  uint8_t switch6 = data[141];
  ESP_LOGD(TAG, "Function switch 6: 0x%02X", switch6);
  if (switch6 & 0x01)
    ESP_LOGD(TAG, "  - Charging high voltage protection enabled");
  if (switch6 & 0x02)
    ESP_LOGD(TAG, "  - Intermittent power supply function enabled");
  if (switch6 & 0x04)
    ESP_LOGD(TAG, "  - Remaining capacity alarm enabled");
  if (switch6 & 0x08)
    ESP_LOGD(TAG, "  - Remaining capacity protection enabled");
  if (switch6 & 0x10)
    ESP_LOGD(TAG, "  - Battery cell low voltage charging prohibited enabled");
  if (switch6 & 0x20)
    ESP_LOGD(TAG, "  - Output reverse polarity protection enabled");
  if (switch6 & 0x40)
    ESP_LOGD(TAG, "  - Output connection failure enabled");
  if (switch6 & 0x80)
    ESP_LOGD(TAG, "  - Output soft start function enabled");

  uint8_t switch7 = data[142];
  ESP_LOGD(TAG, "Function switch 7: 0x%02X", switch7);
  if (switch7 & 0x01)
    ESP_LOGD(TAG, "  - Charge balancing function enabled");
  if (switch7 & 0x02)
    ESP_LOGD(TAG, "  - Static equalization function enabled");
  if (switch7 & 0x04)
    ESP_LOGD(TAG, "  - Timeout prohibits equalization enabled");
  if (switch7 & 0x08)
    ESP_LOGD(TAG, "  - Over-temperature prohibition equalization enabled");
  if (switch7 & 0x10)
    ESP_LOGD(TAG, "  - Automatic activation of charging enabled");
  if (switch7 & 0x20)
    ESP_LOGD(TAG, "  - Manual activation of charging enabled");
  if (switch7 & 0x40)
    ESP_LOGD(TAG, "  - Active current limiting charging enabled");
  if (switch7 & 0x80)
    ESP_LOGD(TAG, "  - Passive current limiting charging enabled");

  uint8_t switch8 = data[143];
  ESP_LOGD(TAG, "Function switch 8: 0x%02X", switch8);
  if (switch8 & 0x01)
    ESP_LOGD(TAG, "  - Switch shutdown function enabled");
  if (switch8 & 0x02)
    ESP_LOGD(TAG, "  - Standby power-off function enabled");
  if (switch8 & 0x04)
    ESP_LOGD(TAG, "  - History function enabled");
  if (switch8 & 0x08)
    ESP_LOGD(TAG, "  - LCD display function enabled");
  if (switch8 & 0x10)
    ESP_LOGD(TAG, "  - Bluetooth communication function enabled");
  if (switch8 & 0x20)
    ESP_LOGD(TAG, "  - Automatic address encoding enabled");
  if (switch8 & 0x40)
    ESP_LOGD(TAG, "  - Parallel external polling enabled");
  if (switch8 & 0x80)
    ESP_LOGD(TAG, "  - Standalone 1.0C charging enabled");
}

void SeplosBmsBle::decode_parallel_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGI(TAG, "Parallel data frame (%zu bytes) received", data.size());
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  if (data.size() < 58) {
    ESP_LOGW(TAG, "Parallel data frame too short (%d bytes)", data.size());
    return;
  }

  // uint16_t data_len = seplos_get_16bit(5);

  ESP_LOGD(TAG, "Data flag: 0x%02X", data[7]);
  ESP_LOGD(TAG, "Device address: %d", data[8]);
  ESP_LOGD(TAG, "Number of cells: %d", data[9]);
  ESP_LOGD(TAG, "Max cell voltage: %.3f V", seplos_get_16bit(10) * 0.001f);
  ESP_LOGD(TAG, "Min cell voltage: %.3f V", seplos_get_16bit(12) * 0.001f);
  ESP_LOGD(TAG, "Temperature quantity: %d", data[14]);
  ESP_LOGD(TAG, "Max cell temperature: %.1f °C", seplos_get_16bit(15) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Min cell temperature: %.1f °C", seplos_get_16bit(17) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Ambient temperature: %.1f °C", seplos_get_16bit(19) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Power temperature: %.1f °C", seplos_get_16bit(21) * 0.1f - 273.15f);
  ESP_LOGD(TAG, "Current: %.1f A", (int16_t) seplos_get_16bit(23) * 0.1f);
  ESP_LOGD(TAG, "Total voltage: %.2f V", seplos_get_16bit(25) * 0.01f);
  ESP_LOGD(TAG, "Remaining capacity: %.1f Ah", seplos_get_16bit(27) * 0.1f);
  ESP_LOGD(TAG, "Custom amount K: %d", data[29]);
  ESP_LOGD(TAG, "Battery capacity: %.1f Ah", seplos_get_16bit(30) * 0.1f);
  ESP_LOGD(TAG, "State of charge: %.1f %%", seplos_get_16bit(32) * 0.1f);
  ESP_LOGD(TAG, "Rated capacity: %.1f Ah", seplos_get_16bit(34) * 0.1f);
  ESP_LOGD(TAG, "Cycles: %d", seplos_get_16bit(36));
  ESP_LOGD(TAG, "State of health: %.1f %%", seplos_get_16bit(38) * 0.1f);
  ESP_LOGD(TAG, "Port voltage: %.2f V", seplos_get_16bit(40) * 0.01f);
  ESP_LOGD(TAG, "Parallel connection status: 0x%04X", seplos_get_16bit(42));

  // System status - see details13
  uint8_t system_status = data[44];
  ESP_LOGD(TAG, "System status: 0x%02X", system_status);
  ESP_LOGD(TAG, "  Bit0 Discharge: %s", ONOFF(system_status & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charge: %s", ONOFF(system_status & 0x02));
  ESP_LOGD(TAG, "  Bit2 Float charge: %s", ONOFF(system_status & 0x04));
  ESP_LOGD(TAG, "  Bit3 Reserved: %s", ONOFF(system_status & 0x08));
  ESP_LOGD(TAG, "  Bit4 Standby: %s", ONOFF(system_status & 0x10));
  ESP_LOGD(TAG, "  Bit5 Shutdown: %s", ONOFF(system_status & 0x20));
  ESP_LOGD(TAG, "  Bit6 Reserved: %s", ONOFF(system_status & 0x40));
  ESP_LOGD(TAG, "  Bit7 Reserved: %s", ONOFF(system_status & 0x80));

  // Switch status - see details14
  uint8_t switch_status = data[45];
  ESP_LOGD(TAG, "Switch status: 0x%02X", switch_status);
  ESP_LOGD(TAG, "  Bit0 Discharge switch: %s", ONOFF(switch_status & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charging switch: %s", ONOFF(switch_status & 0x02));
  ESP_LOGD(TAG, "  Bit2 Current limit switch: %s", ONOFF(switch_status & 0x04));
  ESP_LOGD(TAG, "  Bit3 Heating switch: %s", ONOFF(switch_status & 0x08));
  ESP_LOGD(TAG, "  Bit4 Reserved: %s", ONOFF(switch_status & 0x10));
  ESP_LOGD(TAG, "  Bit5 Reserved: %s", ONOFF(switch_status & 0x20));
  ESP_LOGD(TAG, "  Bit6 Reserved: %s", ONOFF(switch_status & 0x40));
  ESP_LOGD(TAG, "  Bit7 Reserved: %s", ONOFF(switch_status & 0x80));

  // Custom alarm volume P
  uint8_t custom_alarm_volume = data[46];
  ESP_LOGD(TAG, "Custom alarm volume P: %d", custom_alarm_volume);

  // Alarm event1 - Hardware failures
  uint8_t alarm_event1 = data[47];
  ESP_LOGD(TAG, "Alarm event 1: 0x%02X", alarm_event1);
  ESP_LOGD(TAG, "  Bit0 Voltage sensing failure: %s", ONOFF(alarm_event1 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Temperature sensing failure: %s", ONOFF(alarm_event1 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Current sensing failure: %s", ONOFF(alarm_event1 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Key switch failure: %s", ONOFF(alarm_event1 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Cell voltage difference failure: %s", ONOFF(alarm_event1 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Charging switch failed: %s", ONOFF(alarm_event1 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Discharge switch failure: %s", ONOFF(alarm_event1 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Current limit switch failure: %s", ONOFF(alarm_event1 & 0x80));

  // Alarm event2 - Voltage alarms
  uint8_t alarm_event2 = data[48];
  ESP_LOGD(TAG, "Alarm event 2: 0x%02X", alarm_event2);
  ESP_LOGD(TAG, "  Bit0 Single high voltage alarm: %s", ONOFF(alarm_event2 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Single unit overvoltage protection: %s", ONOFF(alarm_event2 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Single unit low voltage alarm: %s", ONOFF(alarm_event2 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Single unit under voltage protection: %s", ONOFF(alarm_event2 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Total pressure high pressure alarm: %s", ONOFF(alarm_event2 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Total voltage overvoltage protection: %s", ONOFF(alarm_event2 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Low total pressure alarm: %s", ONOFF(alarm_event2 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Total voltage undervoltage protection: %s", ONOFF(alarm_event2 & 0x80));

  // Alarm event3 - Cell temperature
  uint8_t alarm_event3 = data[49];
  ESP_LOGD(TAG, "Alarm event 3: 0x%02X", alarm_event3);
  ESP_LOGD(TAG, "  Bit0 Charging high temperature alarm: %s", ONOFF(alarm_event3 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charging over-temperature protection: %s", ONOFF(alarm_event3 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Charging low temperature alarm: %s", ONOFF(alarm_event3 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Charging under-temperature protection: %s", ONOFF(alarm_event3 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Discharge high temperature alarm: %s", ONOFF(alarm_event3 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Discharge over temperature protection: %s", ONOFF(alarm_event3 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Discharge low temperature alarm: %s", ONOFF(alarm_event3 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Discharge under-temperature protection: %s", ONOFF(alarm_event3 & 0x80));

  // Alarm event4 - Environmental/power temperature
  uint8_t alarm_event4 = data[50];
  ESP_LOGD(TAG, "Alarm event 4: 0x%02X", alarm_event4);
  ESP_LOGD(TAG, "  Bit0 Environmental high temperature alarm: %s", ONOFF(alarm_event4 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Environmental over-temperature protection: %s", ONOFF(alarm_event4 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Environmental low temperature alarm: %s", ONOFF(alarm_event4 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Environmental under-temperature protection: %s", ONOFF(alarm_event4 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Power over temperature protection: %s", ONOFF(alarm_event4 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Power high temperature alarm: %s", ONOFF(alarm_event4 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Battery core low temperature heating: %s", ONOFF(alarm_event4 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Secondary trip protection: %s", ONOFF(alarm_event4 & 0x80));

  // Alarm event5 - Current protection
  uint8_t alarm_event5 = data[51];
  ESP_LOGD(TAG, "Alarm event 5: 0x%02X", alarm_event5);
  ESP_LOGD(TAG, "  Bit0 Charging overcurrent alarm: %s", ONOFF(alarm_event5 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charging overcurrent protection: %s", ONOFF(alarm_event5 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Discharge overcurrent alarm: %s", ONOFF(alarm_event5 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Discharge overcurrent protection: %s", ONOFF(alarm_event5 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Transient overcurrent protection: %s", ONOFF(alarm_event5 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Output short circuit protection: %s", ONOFF(alarm_event5 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Transient overcurrent lockout: %s", ONOFF(alarm_event5 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Output short circuit lockout: %s", ONOFF(alarm_event5 & 0x80));

  // Alarm event6 - Charging/output protection
  uint8_t alarm_event6 = data[52];
  ESP_LOGD(TAG, "Alarm event 6: 0x%02X", alarm_event6);
  ESP_LOGD(TAG, "  Bit0 Charging high voltage protection: %s", ONOFF(alarm_event6 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Waiting for intermittent power replenishment: %s", ONOFF(alarm_event6 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Remaining capacity alarm: %s", ONOFF(alarm_event6 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Remaining capacity protection: %s", ONOFF(alarm_event6 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Battery cell low voltage charging is prohibited: %s", ONOFF(alarm_event6 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Output reverse polarity protection: %s", ONOFF(alarm_event6 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Output connection failure: %s", ONOFF(alarm_event6 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Internal alarm: %s", ONOFF(alarm_event6 & 0x80));

  // Alarm event7 - Internal/charging waiting
  uint8_t alarm_event7 = data[53];
  ESP_LOGD(TAG, "Alarm event 7: 0x%02X", alarm_event7);
  ESP_LOGD(TAG, "  Bit0 Internal: %s", ONOFF(alarm_event7 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Internal: %s", ONOFF(alarm_event7 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Internal: %s", ONOFF(alarm_event7 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Internal: %s", ONOFF(alarm_event7 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Automatic charging waiting: %s", ONOFF(alarm_event7 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Manual charging waiting: %s", ONOFF(alarm_event7 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Internal: %s", ONOFF(alarm_event7 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Internal: %s", ONOFF(alarm_event7 & 0x80));

  // Alarm event8 - System/calibration failures
  uint8_t alarm_event8 = data[54];
  ESP_LOGD(TAG, "Alarm event 8: 0x%02X", alarm_event8);
  ESP_LOGD(TAG, "  Bit0 EEP storage failure: %s", ONOFF(alarm_event8 & 0x01));
  ESP_LOGD(TAG, "  Bit1 RTC clock failure: %s", ONOFF(alarm_event8 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Voltage calibration not done: %s", ONOFF(alarm_event8 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Current calibration not done: %s", ONOFF(alarm_event8 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Zero point calibration not done: %s", ONOFF(alarm_event8 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Perpetual calendar not synchronized: %s", ONOFF(alarm_event8 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Internal: %s", ONOFF(alarm_event8 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Internal: %s", ONOFF(alarm_event8 & 0x80));

  // Process additional custom alarm events if any (beyond the standard 8)
  if (custom_alarm_volume > 8 && data.size() > 47 + custom_alarm_volume) {
    for (uint8_t i = 8; i < custom_alarm_volume && (47 + i) < data.size(); i++) {
      ESP_LOGD(TAG, "Custom alarm event %d: 0x%02X", i + 1, data[47 + i]);
    }
  }
}

void SeplosBmsBle::decode_single_machine_data_(const std::vector<uint8_t> &data) {
  auto seplos_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGI(TAG, "Status frame (%zu bytes) received", data.size());
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  if (data.size() < 60) {
    ESP_LOGW(TAG, "Status frame too short (%d bytes)", data.size());
    return;
  }

  // uint16_t data_len = seplos_get_16bit(5);
  ESP_LOGD(TAG, "Device address: %d", data[7]);
  ESP_LOGD(TAG, "Reserved byte: %d", data[8]);

  uint8_t cells = data[9];
  uint8_t temperatures = data[7 + 3 + (cells * 2)];
  ESP_LOGD(TAG, "Number of cells: %d", cells);

  if (data.size() < 7 + 3 + (cells * 2) + 1 + (temperatures * 2) + 58 + 2 + 1) {
    ESP_LOGW(TAG, "Status frame too short (%d bytes)", data.size());
    return;
  }

  this->min_cell_voltage_ = 100.0f;
  this->max_cell_voltage_ = -100.0f;
  float total_cell_voltage = 0.0f;

  for (uint8_t i = 0; i < cells && i < 24; i++) {
    float cell_voltage = seplos_get_16bit(7 + 3 + (i * 2)) * 0.001f;
    this->publish_state_(this->cells_[i].cell_voltage_sensor_, cell_voltage);

    total_cell_voltage += cell_voltage;
    if (cell_voltage < this->min_cell_voltage_) {
      this->min_cell_voltage_ = cell_voltage;
      this->min_voltage_cell_ = i + 1;
    }
    if (cell_voltage > this->max_cell_voltage_) {
      this->max_cell_voltage_ = cell_voltage;
      this->max_voltage_cell_ = i + 1;
    }
  }

  this->publish_state_(this->min_cell_voltage_sensor_, this->min_cell_voltage_);
  this->publish_state_(this->max_cell_voltage_sensor_, this->max_cell_voltage_);
  this->publish_state_(this->min_voltage_cell_sensor_, this->min_voltage_cell_);
  this->publish_state_(this->max_voltage_cell_sensor_, this->max_voltage_cell_);
  this->publish_state_(this->delta_cell_voltage_sensor_, this->max_cell_voltage_ - this->min_cell_voltage_);
  if (cells > 0) {
    this->publish_state_(this->average_cell_voltage_sensor_, total_cell_voltage / cells);
  }

  size_t offset = 7 + 3 + (cells * 2);  // 7 (header) + 3 (device_addr + reserved + cells) + cells

  ESP_LOGD(TAG, "Temperature sensor count: %d", temperatures);

  uint8_t cell_temperatures = temperatures > 2 ? temperatures - 2 : 0;
  float total_cell_temperature = 0.0f;

  for (uint8_t i = 0; i < cell_temperatures && i < 8; i++) {
    float cell_temperature = (seplos_get_16bit(offset + 1 + (i * 2)) - 2731) * 0.1f;
    this->publish_state_(this->temperatures_[i].temperature_sensor_, cell_temperature);
    total_cell_temperature += cell_temperature;
  }

  this->publish_state_(this->ambient_temperature_sensor_,
                       (seplos_get_16bit(offset + 1 + (cell_temperatures * 2)) - 2731) * 0.1f);
  this->publish_state_(this->mosfet_temperature_sensor_,
                       (seplos_get_16bit(offset + 1 + (cell_temperatures * 2) + 2) - 2731) * 0.1f);

  // Calculate average cell temperature (only cell temperatures, not ambient/mosfet)
  if (cell_temperatures > 0) {
    this->publish_state_(this->average_cell_temperature_sensor_, total_cell_temperature / cell_temperatures);
  }

  offset = 7 + 3 + (cells * 2) + 1 + (temperatures * 2);

  float current = (int16_t) seplos_get_16bit(offset + 0) * 0.01f;
  this->publish_state_(this->current_sensor_, current);

  float total_voltage = seplos_get_16bit(offset + 2) * 0.01f;
  this->publish_state_(this->total_voltage_sensor_, total_voltage);

  float power = total_voltage * current;
  this->publish_state_(this->power_sensor_, power);
  this->publish_state_(this->charging_power_sensor_, power > 0 ? power : 0);
  this->publish_state_(this->discharging_power_sensor_, power < 0 ? -power : 0);

  this->publish_state_(this->charging_binary_sensor_, current > 0.1f);
  this->publish_state_(this->discharging_binary_sensor_, current < -0.1f);

  this->publish_state_(this->capacity_remaining_sensor_, seplos_get_16bit(offset + 4) * 0.01f);

  // Reserved: 1 byte - offset + 6 (skip)

  this->publish_state_(this->battery_capacity_sensor_, seplos_get_16bit(offset + 7) * 0.01f);
  this->publish_state_(this->state_of_charge_sensor_, seplos_get_16bit(offset + 9) * 0.1f);
  this->publish_state_(this->nominal_capacity_sensor_, seplos_get_16bit(offset + 11) * 0.01f);
  this->publish_state_(this->charging_cycles_sensor_, seplos_get_16bit(offset + 13));
  this->publish_state_(this->state_of_health_sensor_, seplos_get_16bit(offset + 15) * 0.1f);
  this->publish_state_(this->port_voltage_sensor_, seplos_get_16bit(offset + 17) * 0.01f);

  offset = 7 + 3 + (cells * 2) + 1 + (temperatures * 2) + 19;

  // Cell voltage alarms: 1 byte each for M cells - starting at offset + 0
  for (uint8_t i = 0; i < cells && i < 24; i++) {
    ESP_LOGD(TAG, "Cell %d voltage alarm: %d", i + 1, data[offset + i]);
  }

  // Cell temperature alarms: 1 byte each for (N-2) cell temperatures - starting at offset + cells
  for (uint8_t i = 0; i < cell_temperatures && i < 8; i++) {
    ESP_LOGD(TAG, "Cell %d temperature alarm: %d", i + 1, data[offset + cells + i]);
  }

  ESP_LOGD(TAG, "Ambient temperature alarm: %d", data[offset + cells + temperatures - 2]);

  ESP_LOGD(TAG, "Mosfet temperature alarm: %d", data[offset + cells + temperatures - 1]);

  size_t protection_offset = offset + cells + temperatures;

  ESP_LOGD(TAG, "Charge/discharge current alarm: 0x%02X", data[protection_offset + 0]);

  ESP_LOGD(TAG, "Battery total voltage alarm: 0x%02X", data[protection_offset + 1]);

  // System status - see details13
  uint8_t system_status = data[protection_offset + 2];
  ESP_LOGD(TAG, "System status: 0x%02X", system_status);
  ESP_LOGD(TAG, "  Bit0 Discharge: %s", ONOFF(system_status & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charge: %s", ONOFF(system_status & 0x02));
  ESP_LOGD(TAG, "  Bit2 Float charge: %s", ONOFF(system_status & 0x04));
  ESP_LOGD(TAG, "  Bit3 Reserved: %s", ONOFF(system_status & 0x08));
  ESP_LOGD(TAG, "  Bit4 Standby: %s", ONOFF(system_status & 0x10));
  ESP_LOGD(TAG, "  Bit5 Shutdown: %s", ONOFF(system_status & 0x20));
  ESP_LOGD(TAG, "  Bit6 Reserved: %s", ONOFF(system_status & 0x40));
  ESP_LOGD(TAG, "  Bit7 Reserved: %s", ONOFF(system_status & 0x80));

  // Switch status - see details14
  uint8_t switch_status = data[protection_offset + 3];
  ESP_LOGD(TAG, "Switch status: 0x%02X", switch_status);
  ESP_LOGD(TAG, "  Bit0 Discharge switch: %s", ONOFF(switch_status & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charging switch: %s", ONOFF(switch_status & 0x02));
  ESP_LOGD(TAG, "  Bit2 Current limit switch: %s", ONOFF(switch_status & 0x04));
  ESP_LOGD(TAG, "  Bit3 Heating switch: %s", ONOFF(switch_status & 0x08));
  ESP_LOGD(TAG, "  Bit4 Reserved: %s", ONOFF(switch_status & 0x10));
  ESP_LOGD(TAG, "  Bit5 Reserved: %s", ONOFF(switch_status & 0x20));
  ESP_LOGD(TAG, "  Bit6 Reserved: %s", ONOFF(switch_status & 0x40));
  ESP_LOGD(TAG, "  Bit7 Reserved: %s", ONOFF(switch_status & 0x80));

  this->publish_state_(this->discharging_switch_, switch_status & 0x01);
  this->publish_state_(this->charging_switch_, switch_status & 0x02);
  this->publish_state_(this->current_limit_switch_, switch_status & 0x04);
  this->publish_state_(this->heating_switch_, switch_status & 0x08);

  // Custom alarm volume P (1 byte)
  uint8_t custom_alarm_volume = data[protection_offset + 4];
  ESP_LOGD(TAG, "Custom alarm volume P: %d", custom_alarm_volume);

  size_t alarm_offset = protection_offset + 5;

  // Alarm event1 - Hardware failures
  uint8_t alarm_event1 = data[alarm_offset + 0];
  ESP_LOGD(TAG, "Alarm event 1: 0x%02X", alarm_event1);
  ESP_LOGD(TAG, "  Bit0 Voltage sensing failure: %s", ONOFF(alarm_event1 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Temperature sensing failure: %s", ONOFF(alarm_event1 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Current sensing failure: %s", ONOFF(alarm_event1 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Key switch failure: %s", ONOFF(alarm_event1 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Cell voltage difference failure: %s", ONOFF(alarm_event1 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Charging switch failed: %s", ONOFF(alarm_event1 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Discharge switch failure: %s", ONOFF(alarm_event1 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Current limit switch failure: %s", ONOFF(alarm_event1 & 0x80));
  this->publish_state_(this->alarm_event1_bitmask_sensor_, (float) alarm_event1);

  // Alarm event2 - Voltage alarms
  uint8_t alarm_event2 = data[alarm_offset + 1];
  ESP_LOGD(TAG, "Alarm event 2: 0x%02X", alarm_event2);
  ESP_LOGD(TAG, "  Bit0 Single high voltage alarm: %s", ONOFF(alarm_event2 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Single unit overvoltage protection: %s", ONOFF(alarm_event2 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Single unit low voltage alarm: %s", ONOFF(alarm_event2 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Single unit under voltage protection: %s", ONOFF(alarm_event2 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Total pressure high pressure alarm: %s", ONOFF(alarm_event2 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Total voltage overvoltage protection: %s", ONOFF(alarm_event2 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Low total pressure alarm: %s", ONOFF(alarm_event2 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Total voltage undervoltage protection: %s", ONOFF(alarm_event2 & 0x80));
  this->publish_state_(this->alarm_event2_bitmask_sensor_, (float) alarm_event2);

  // Alarm event3 - Cell temperature
  uint8_t alarm_event3 = data[alarm_offset + 2];
  ESP_LOGD(TAG, "Alarm event 3: 0x%02X", alarm_event3);
  ESP_LOGD(TAG, "  Bit0 Charging high temperature alarm: %s", ONOFF(alarm_event3 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charging over-temperature protection: %s", ONOFF(alarm_event3 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Charging low temperature alarm: %s", ONOFF(alarm_event3 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Charging under-temperature protection: %s", ONOFF(alarm_event3 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Discharge high temperature alarm: %s", ONOFF(alarm_event3 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Discharge over temperature protection: %s", ONOFF(alarm_event3 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Discharge low temperature alarm: %s", ONOFF(alarm_event3 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Discharge under-temperature protection: %s", ONOFF(alarm_event3 & 0x80));
  this->publish_state_(this->alarm_event3_bitmask_sensor_, (float) alarm_event3);

  // Alarm event4 - Environmental/power temperature
  uint8_t alarm_event4 = data[alarm_offset + 3];
  ESP_LOGD(TAG, "Alarm event 4: 0x%02X", alarm_event4);
  ESP_LOGD(TAG, "  Bit0 Environmental high temperature alarm: %s", ONOFF(alarm_event4 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Environmental over-temperature protection: %s", ONOFF(alarm_event4 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Environmental low temperature alarm: %s", ONOFF(alarm_event4 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Environmental under-temperature protection: %s", ONOFF(alarm_event4 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Power over temperature protection: %s", ONOFF(alarm_event4 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Power high temperature alarm: %s", ONOFF(alarm_event4 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Battery core low temperature heating: %s", ONOFF(alarm_event4 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Secondary trip protection: %s", ONOFF(alarm_event4 & 0x80));
  this->publish_state_(this->alarm_event4_bitmask_sensor_, (float) alarm_event4);

  // Alarm event5 - Current protection
  uint8_t alarm_event5 = data[alarm_offset + 4];
  ESP_LOGD(TAG, "Alarm event 5: 0x%02X", alarm_event5);
  ESP_LOGD(TAG, "  Bit0 Charging overcurrent alarm: %s", ONOFF(alarm_event5 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Charging overcurrent protection: %s", ONOFF(alarm_event5 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Discharge overcurrent alarm: %s", ONOFF(alarm_event5 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Discharge overcurrent protection: %s", ONOFF(alarm_event5 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Transient overcurrent protection: %s", ONOFF(alarm_event5 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Output short circuit protection: %s", ONOFF(alarm_event5 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Transient overcurrent lockout: %s", ONOFF(alarm_event5 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Output short circuit lockout: %s", ONOFF(alarm_event5 & 0x80));
  this->publish_state_(this->alarm_event5_bitmask_sensor_, (float) alarm_event5);

  // Alarm event6 - Charging/output protection
  uint8_t alarm_event6 = data[alarm_offset + 5];
  ESP_LOGD(TAG, "Alarm event 6: 0x%02X", alarm_event6);
  ESP_LOGD(TAG, "  Bit0 Charging high voltage protection: %s", ONOFF(alarm_event6 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Waiting for intermittent power replenishment: %s", ONOFF(alarm_event6 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Remaining capacity alarm: %s", ONOFF(alarm_event6 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Remaining capacity protection: %s", ONOFF(alarm_event6 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Battery cell low voltage charging is prohibited: %s", ONOFF(alarm_event6 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Output reverse polarity protection: %s", ONOFF(alarm_event6 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Output connection failure: %s", ONOFF(alarm_event6 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Internal alarm: %s", ONOFF(alarm_event6 & 0x80));
  this->publish_state_(this->alarm_event6_bitmask_sensor_, (float) alarm_event6);

  // Alarm event7 - Internal/charging waiting
  uint8_t alarm_event7 = data[alarm_offset + 6];
  ESP_LOGD(TAG, "Alarm event 7: 0x%02X", alarm_event7);
  ESP_LOGD(TAG, "  Bit0 Internal: %s", ONOFF(alarm_event7 & 0x01));
  ESP_LOGD(TAG, "  Bit1 Internal: %s", ONOFF(alarm_event7 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Internal: %s", ONOFF(alarm_event7 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Internal: %s", ONOFF(alarm_event7 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Automatic charging waiting: %s", ONOFF(alarm_event7 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Manual charging waiting: %s", ONOFF(alarm_event7 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Internal: %s", ONOFF(alarm_event7 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Internal: %s", ONOFF(alarm_event7 & 0x80));
  this->publish_state_(this->alarm_event7_bitmask_sensor_, (float) alarm_event7);

  // Alarm event8 - System/calibration failures
  uint8_t alarm_event8 = data[alarm_offset + 7];
  ESP_LOGD(TAG, "Alarm event 8: 0x%02X", alarm_event8);
  ESP_LOGD(TAG, "  Bit0 EEP storage failure: %s", ONOFF(alarm_event8 & 0x01));
  ESP_LOGD(TAG, "  Bit1 RTC clock failure: %s", ONOFF(alarm_event8 & 0x02));
  ESP_LOGD(TAG, "  Bit2 Voltage calibration not done: %s", ONOFF(alarm_event8 & 0x04));
  ESP_LOGD(TAG, "  Bit3 Current calibration not done: %s", ONOFF(alarm_event8 & 0x08));
  ESP_LOGD(TAG, "  Bit4 Zero point calibration not done: %s", ONOFF(alarm_event8 & 0x10));
  ESP_LOGD(TAG, "  Bit5 Perpetual calendar not synchronized: %s", ONOFF(alarm_event8 & 0x20));
  ESP_LOGD(TAG, "  Bit6 Internal: %s", ONOFF(alarm_event8 & 0x40));
  ESP_LOGD(TAG, "  Bit7 Internal: %s", ONOFF(alarm_event8 & 0x80));
  this->publish_state_(this->alarm_event8_bitmask_sensor_, (float) alarm_event8);

  std::string consolidated_alarms = this->decode_all_alarm_events_(
      alarm_event1, alarm_event2, alarm_event3, alarm_event4, alarm_event5, alarm_event6, alarm_event7, alarm_event8);
  this->publish_state_(this->alarms_text_sensor_, consolidated_alarms);

  // Process additional custom alarm events if any (beyond the standard 8)
  if (custom_alarm_volume > 8) {
    for (uint8_t i = 8; i < custom_alarm_volume && (alarm_offset + i) < data.size(); i++) {
      ESP_LOGD(TAG, "Custom alarm event %d: 0x%02X", i + 1, data[alarm_offset + i]);
    }
  }

  // Balancing states (M/8 bytes where M is number of cells)
  size_t balancing_offset = alarm_offset + custom_alarm_volume;
  uint8_t balancing_bytes = (cells + 7) / 8;  // Round up to next byte
  ESP_LOGD(TAG, "Balancing states (%d bytes):", balancing_bytes);

  for (uint8_t i = 0; i < balancing_bytes; i++) {
    uint8_t balancing_state = data[balancing_offset + i];
    ESP_LOGD(TAG, "  Balancing state byte %d: 0x%02X", i + 1, balancing_state);

    for (uint8_t bit = 0; bit < 8; bit++) {
      uint8_t cell = i * 8 + bit + 1;
      if (cell <= cells && (balancing_state & (1 << bit))) {
        ESP_LOGD(TAG, "    - Cell %d balancing", cell);
      }
    }
  }

  // Disconnected states (M/8 bytes where M is number of cells)
  size_t disconnected_offset = balancing_offset + balancing_bytes;
  uint8_t disconnected_bytes = (cells + 7) / 8;  // Round up to next byte
  ESP_LOGD(TAG, "Disconnected states (%d bytes):", disconnected_bytes);

  for (uint8_t i = 0; i < disconnected_bytes; i++) {
    uint8_t disconnected_state = data[disconnected_offset + i];
    ESP_LOGD(TAG, "  Disconnected state byte %d: 0x%02X", i + 1, disconnected_state);

    for (uint8_t bit = 0; bit < 8; bit++) {
      uint8_t cell = i * 8 + bit + 1;
      if (cell <= cells && (disconnected_state & (1 << bit))) {
        ESP_LOGD(TAG, "    - Cell %d disconnected", cell);
      }
    }
  }

  if (protection_offset + 24 < data.size()) {
    ESP_LOGD(TAG, "Remaining bytes: %s",
             format_hex_pretty(&data[protection_offset + 24], data.size() - protection_offset - 24 - 3).c_str());
  }
}

void SeplosBmsBle::dump_config() {  // NOLINT(google-readability-function-size,readability-function-size)
  ESP_LOGCONFIG(TAG, "SeplosBmsBle:");

  LOG_BINARY_SENSOR("", "Charging", this->charging_binary_sensor_);
  LOG_BINARY_SENSOR("", "Discharging", this->discharging_binary_sensor_);
  LOG_BINARY_SENSOR("", "Limiting current", this->limiting_current_binary_sensor_);
  LOG_BINARY_SENSOR("", "Online status", this->online_status_binary_sensor_);

  LOG_SENSOR("", "Total voltage", this->total_voltage_sensor_);
  LOG_SENSOR("", "Current", this->current_sensor_);
  LOG_SENSOR("", "Power", this->power_sensor_);
  LOG_SENSOR("", "Charging power", this->charging_power_sensor_);
  LOG_SENSOR("", "Discharging power", this->discharging_power_sensor_);
  LOG_SENSOR("", "Capacity remaining", this->capacity_remaining_sensor_);
  LOG_SENSOR("", "Voltage protection bitmask", this->voltage_protection_bitmask_sensor_);
  LOG_SENSOR("", "Current protection bitmask", this->current_protection_bitmask_sensor_);
  LOG_SENSOR("", "Temperature protection bitmask", this->temperature_protection_bitmask_sensor_);
  LOG_SENSOR("", "State of charge", this->state_of_charge_sensor_);
  LOG_SENSOR("", "Nominal capacity", this->nominal_capacity_sensor_);
  LOG_SENSOR("", "Charging cycles", this->charging_cycles_sensor_);
  LOG_SENSOR("", "Min cell voltage", this->min_cell_voltage_sensor_);
  LOG_SENSOR("", "Max cell voltage", this->max_cell_voltage_sensor_);
  LOG_SENSOR("", "Min voltage cell", this->min_voltage_cell_sensor_);
  LOG_SENSOR("", "Max voltage cell", this->max_voltage_cell_sensor_);
  LOG_SENSOR("", "Delta cell voltage", this->delta_cell_voltage_sensor_);
  LOG_SENSOR("", "Average cell temperature", this->average_cell_temperature_sensor_);
  LOG_SENSOR("", "Ambient temperature", this->ambient_temperature_sensor_);
  LOG_SENSOR("", "Mosfet temperature", this->mosfet_temperature_sensor_);
  LOG_SENSOR("", "State of health", this->state_of_health_sensor_);
  LOG_SENSOR("", "Port voltage", this->port_voltage_sensor_);
  LOG_SENSOR("", "Battery capacity", this->battery_capacity_sensor_);
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
  LOG_SENSOR("", "Alarm Event 1 Bitmask", this->alarm_event1_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 2 Bitmask", this->alarm_event2_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 3 Bitmask", this->alarm_event3_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 4 Bitmask", this->alarm_event4_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 5 Bitmask", this->alarm_event5_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 6 Bitmask", this->alarm_event6_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 7 Bitmask", this->alarm_event7_bitmask_sensor_);
  LOG_SENSOR("", "Alarm Event 8 Bitmask", this->alarm_event8_bitmask_sensor_);

  LOG_TEXT_SENSOR("", "Device model", this->device_model_text_sensor_);
  LOG_TEXT_SENSOR("", "Hardware version", this->hardware_version_text_sensor_);
  LOG_TEXT_SENSOR("", "Software version", this->software_version_text_sensor_);
  LOG_TEXT_SENSOR("", "Voltage protection", this->voltage_protection_text_sensor_);
  LOG_TEXT_SENSOR("", "Current protection", this->current_protection_text_sensor_);
  LOG_TEXT_SENSOR("", "Temperature protection", this->temperature_protection_text_sensor_);
  LOG_TEXT_SENSOR("", "Alarms", this->alarms_text_sensor_);
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

void SeplosBmsBle::publish_state_(switch_::Switch *obj, const bool &state) {
  if (obj == nullptr)
    return;

  obj->publish_state(state);
}

bool SeplosBmsBle::send_command(uint8_t function, const std::vector<uint8_t> &payload) {
  std::vector<uint8_t> data;
  data.push_back(0x10);      // VER
  data.push_back(0x00);      // ADDR
  data.push_back(0x46);      // CID1
  data.push_back(function);  // CID2

  // Length field (big-endian)
  uint16_t payload_length = payload.size();
  data.push_back(payload_length >> 8);    // LEN high
  data.push_back(payload_length & 0xFF);  // LEN low

  // Add payload if present
  data.insert(data.end(), payload.begin(), payload.end());

  auto crc = crc_xmodem(data.data(), data.size());
  data.push_back(crc >> 8);
  data.push_back(crc >> 0);

  data.insert(data.begin(), 0x7e);  // SOF
  data.push_back(0x0d);             // EOF (0x0D)

  ESP_LOGD(TAG, "Send command (handle 0x%02X): %s", this->char_command_handle_,
           format_hex_pretty(&data.front(), data.size()).c_str());

  auto status =
      esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), this->char_command_handle_,
                               data.size(), &data.front(), ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);

  if (status) {
    ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status=%d", this->parent_->address_str().c_str(), status);
  }

  return (status == 0);
}

std::string SeplosBmsBle::bitmask_to_string_(const char *const messages[], const uint8_t &messages_size,
                                             const uint8_t &mask) {
  std::string values = "";
  if (mask) {
    for (uint8_t i = 0; i < messages_size; i++) {
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

std::string SeplosBmsBle::decode_all_alarm_events_(uint8_t alarm_event1, uint8_t alarm_event2, uint8_t alarm_event3,
                                                   uint8_t alarm_event4, uint8_t alarm_event5, uint8_t alarm_event6,
                                                   uint8_t alarm_event7, uint8_t alarm_event8) {
  std::string all_alarms = "";

  uint8_t alarm_events[8] = {alarm_event1, alarm_event2, alarm_event3, alarm_event4,
                             alarm_event5, alarm_event6, alarm_event7, alarm_event8};

  const char *const *alarm_messages[8] = {ALARM_EVENT1_MESSAGES, ALARM_EVENT2_MESSAGES, ALARM_EVENT3_MESSAGES,
                                          ALARM_EVENT4_MESSAGES, ALARM_EVENT5_MESSAGES, ALARM_EVENT6_MESSAGES,
                                          ALARM_EVENT7_MESSAGES, ALARM_EVENT8_MESSAGES};

  for (uint8_t event = 0; event < 8; event++) {
    if (alarm_events[event]) {
      std::string event_alarms = this->bitmask_to_string_(alarm_messages[event], 8, alarm_events[event]);
      if (!event_alarms.empty()) {
        if (!all_alarms.empty()) {
          all_alarms.append(";");
        }
        all_alarms.append(event_alarms);
      }
    }
  }

  if (all_alarms.empty()) {
    all_alarms = "No alarms";
  }

  return all_alarms;
}

}  // namespace seplos_bms_ble
}  // namespace esphome
