#include "seplos_modbus.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace seplos_modbus {

static const char *const TAG = "seplos_modbus";

void SeplosModbus::setup() {
  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->setup();
  }
}
void SeplosModbus::loop() {
  const uint32_t now = millis();

  if (now - this->last_seplos_modbus_byte_ > 50) {
    this->rx_buffer_.clear();
    this->last_seplos_modbus_byte_ = now;
  }

  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    if (this->parse_seplos_modbus_byte_(byte)) {
      this->last_seplos_modbus_byte_ = now;
    } else {
      this->rx_buffer_.clear();
    }
  }
}

uint16_t chksum(const uint8_t data[], const uint16_t len) {
  uint16_t checksum = 0x00;
  for (uint16_t i = 0; i < len; i++) {
    checksum = checksum + data[i];
  }
  checksum = ~checksum;
  checksum += 1;
  return checksum;
}

uint16_t lchksum(const uint16_t len) {
  uint16_t lchecksum = 0x0000;

  if (len == 0)
    return 0x0000;

  lchecksum = (len & 0xf) + ((len >> 4) & 0xf) + ((len >> 8) & 0xf);
  lchecksum = ~(lchecksum % 16) + 1;

  return (lchecksum << 12) + len;  // 4 byte checksum + 12 bytes length
}

uint8_t ascii_hex_to_byte(char a, char b) {
  a = (a <= '9') ? a - '0' : (a & 0x7) + 9;
  b = (b <= '9') ? b - '0' : (b & 0x7) + 9;

  return (a << 4) + b;
}

bool SeplosModbus::parse_seplos_modbus_byte_(uint8_t byte) {
  size_t at = this->rx_buffer_.size();
  this->rx_buffer_.push_back(byte);
  const uint8_t *raw = &this->rx_buffer_[0];

  if (at == 0)
    return true;

  // Start of frame
  if (raw[0] != 0x7E) {
    ESP_LOGW(TAG, "Invalid header.");

    // return false to reset buffer
    return false;
  }

  // End of frame '\r'
  if (raw[at] != 0x0D)
    return true;

  uint16_t data_len = at - 4 - 1;
  uint16_t computed_crc = chksum(raw + 1, data_len);
  uint16_t remote_crc = uint16_t(ascii_hex_to_byte(raw[at - 4], raw[at - 3])) << 8 |
                        (uint16_t(ascii_hex_to_byte(raw[at - 2], raw[at - 1])) << 0);
  if (computed_crc != remote_crc) {
    ESP_LOGW(TAG, "SeplosBms CRC Check failed! %04X != %04X", computed_crc, remote_crc);
    return false;
  }

  std::vector<uint8_t> data;
  for (uint16_t i = 1; i < data_len; i = i + 2) {
    data.push_back(ascii_hex_to_byte(raw[i], raw[i + 1]));
  }

  uint8_t address = data[1];

  bool found = false;
  for (auto *device : this->devices_) {
    if (device->address_ == address) {
      device->on_seplos_modbus_data(data);
      found = true;
    }
  }

  if (!found) {
    ESP_LOGW(TAG, "Got SeplosModbus frame from unknown address 0x%02X! ", address);
  }

  // return false to reset buffer
  return false;
}

void SeplosModbus::dump_config() {
  ESP_LOGCONFIG(TAG, "SeplosModbus:");
  LOG_PIN("  Flow Control Pin: ", this->flow_control_pin_);
}
float SeplosModbus::get_setup_priority() const {
  // After UART bus
  return setup_priority::BUS - 1.0f;
}

void SeplosModbus::send(uint8_t address) {
  if (this->flow_control_pin_ != nullptr)
    this->flow_control_pin_->digital_write(true);

  this->write_str("~20004642E00200FD37\r");
  this->flush();

  if (this->flow_control_pin_ != nullptr)
    this->flow_control_pin_->digital_write(false);
}

}  // namespace seplos_modbus
}  // namespace esphome
