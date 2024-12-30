#include "seplos_bms_ble.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace seplos_bms_ble {

static const char *const TAG = "seplos_bms_ble";

static const uint16_t SEPLOS_BMS_SERVICE_UUID = 0xFF00;
static const uint16_t SEPLOS_BMS_NOTIFY_CHARACTERISTIC_UUID = 0xFF01;   // handle 0x12
static const uint16_t SEPLOS_BMS_CONTROL_CHARACTERISTIC_UUID = 0xFF02;  // handle 0x14

static const uint16_t MAX_RESPONSE_SIZE = 200;

static const uint16_t SEPLOS_PKT_START = 0x7E;
static const uint16_t SEPLOS_PKT_END = 0x0D;

// 0x47 Settings frame
// 0x51 Manufacturer information
// 0x61 Get single machine data
// 0x62 Get parallel data
// 0x63 switchCANprotocol
// 0x64 switch485protocol
// 0xA1 set upBMSParameter information (protection switch)
// 0x65 Set device group number and name

static const uint8_t SEPLOS_COMMAND_QUEUE_SIZE = 6;
static const uint8_t SEPLOS_COMMAND_QUEUE[SEPLOS_COMMAND_QUEUE_SIZE] = {0x47};

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

      this->send_command_(0x47, 0x00);
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      ESP_LOGV(TAG, "Notification received (handle 0x%02X): %s", param->notify.handle,
               format_hex_pretty(param->notify.value, param->notify.value_len).c_str());

      std::vector<uint8_t> data(param->notify.value, param->notify.value + param->notify.value_len);

      this->on_seplos_bms_ble_data(param->notify.handle, data);
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
  // this->send_command_(SEPLOS_COMMAND_QUEUE[this->next_command_++ % SEPLOS_COMMAND_QUEUE_SIZE]);
  this->send_command_(0x47, 0x00);
}

void SeplosBmsBle::on_seplos_bms_ble_data(const uint8_t &handle, const std::vector<uint8_t> &data) {
  if (data[0] != SEPLOS_PKT_START || data.back() != SEPLOS_PKT_END || data.size() > MAX_RESPONSE_SIZE) {
    ESP_LOGW(TAG, "Invalid response received (length: %d bytes): %s", data.size(),
             format_hex_pretty(&data.front(), data.size()).c_str());
    return;
  }

  uint8_t frame_type = data[3];

  switch (frame_type) {
    case 0x47:
      this->decode_status_data_(data);
      break;
    default:
      ESP_LOGW(TAG, "Unhandled response received (frame_type 0x%02X): %s", frame_type,
               format_hex_pretty(&data.front(), data.size()).c_str());
  }

  // Send next command after each received frame
  if (this->next_command_ < SEPLOS_COMMAND_QUEUE_SIZE) {
    this->next_command_++;
    // this->send_command_(SEPLOS_COMMAND_QUEUE[this->next_command_++ % SEPLOS_COMMAND_QUEUE_SIZE]);
    this->send_command_(0x47, 0x00);
  }
}

void SeplosBmsBle::decode_hardware_version_data_(const std::vector<uint8_t> &data) {
  ESP_LOGI(TAG, "Hardware version frame received");
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Byte Len Payload      Description                      Unit  Precision
  //  0    1  0x55         Start of frame
  //  1    1  0x14         Frame type (Response)
  //  2    1  0x82         Address

  //  3    16 0x54 0x50 0x2d 0x4c 0x54 0x35 0x35 0x00 0x54 0x42 0x00 0x00 0x00 0x00 0x00 0x00    "TP-LT55" "TB"
  // this->publish_state_(this->device_model_text_sensor_, std::string(data.begin() + 3, data.end() - 1));

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

bool SeplosBmsBle::send_command_(uint8_t function, uint8_t value) {
  std::vector<uint8_t> data;
  data.push_back(0x10);      // VER
  data.push_back(0x00);      // ADDR
  data.push_back(0x46);      // CID1
  data.push_back(function);  // CID2 (0x42)
  data.push_back(0x00);      // LEN high
  data.push_back(0x01);      // LEN low
  data.push_back(value);     // VALUE (0x00)

  auto crc = crc16be(data.data(), data.size());
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
