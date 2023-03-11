#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace seplos_modbus {

class SeplosModbusDevice;

class SeplosModbus : public uart::UARTDevice, public Component {
 public:
  SeplosModbus() = default;

  void setup() override;

  void loop() override;

  void dump_config() override;

  void register_device(SeplosModbusDevice *device) { this->devices_.push_back(device); }

  float get_setup_priority() const override;

  void send(uint8_t protocol_version, uint8_t address, uint8_t function, uint8_t value);
  void set_rx_timeout(uint16_t rx_timeout) { rx_timeout_ = rx_timeout; }
  void set_flow_control_pin(GPIOPin *flow_control_pin) { this->flow_control_pin_ = flow_control_pin; }

 protected:
  uint16_t rx_timeout_{150};
  GPIOPin *flow_control_pin_{nullptr};

  bool parse_seplos_modbus_byte_(uint8_t byte);
  std::vector<uint8_t> rx_buffer_;
  uint32_t last_seplos_modbus_byte_{0};
  uint32_t last_send_{0};
  std::vector<SeplosModbusDevice *> devices_;
};

uint16_t crc16(const uint8_t *data, uint8_t len);

class SeplosModbusDevice {
 public:
  void set_parent(SeplosModbus *parent) { parent_ = parent; }
  void set_address(uint8_t address) { address_ = address; }
  void set_protocol_version(uint8_t protocol_version) { protocol_version_ = protocol_version; }
  virtual void on_seplos_modbus_data(const std::vector<uint8_t> &data) = 0;
  void send(uint8_t function, uint8_t value) {
    this->parent_->send(this->protocol_version_, this->address_, function, value);
  }

 protected:
  friend SeplosModbus;

  SeplosModbus *parent_;
  uint8_t address_;
  uint8_t protocol_version_;
};

}  // namespace seplos_modbus
}  // namespace esphome
