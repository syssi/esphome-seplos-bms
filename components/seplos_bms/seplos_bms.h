#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/seplos_modbus/seplos_modbus.h"

namespace esphome {
namespace seplos_bms {

class SeplosBms : public PollingComponent, public seplos_modbus::SeplosModbusDevice {
 public:
  void set_fan_running_binary_sensor(binary_sensor::BinarySensor *fan_running_binary_sensor) {
    fan_running_binary_sensor_ = fan_running_binary_sensor;
  }

  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }

  void set_errors_text_sensor(text_sensor::TextSensor *errors_text_sensor) { errors_text_sensor_ = errors_text_sensor; }

  void on_seplos_modbus_data(const std::vector<uint8_t> &data) override;

  void dump_config() override;
  void update() override;
  float get_setup_priority() const override;

 protected:
  binary_sensor::BinarySensor *fan_running_binary_sensor_;

  sensor::Sensor *temperature_sensor_;

  text_sensor::TextSensor *errors_text_sensor_;

  void publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state);
  void publish_state_(sensor::Sensor *sensor, float value);
  void publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state);
  void on_telemetry_data(const std::vector<uint8_t> &data);
};

}  // namespace seplos_bms
}  // namespace esphome
