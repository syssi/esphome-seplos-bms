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
  void set_online_status_binary_sensor(binary_sensor::BinarySensor *online_status_binary_sensor) {
    online_status_binary_sensor_ = online_status_binary_sensor;
  }

  void set_voltage_protection_binary_sensor(binary_sensor::BinarySensor *voltage_protection_binary_sensor) {
    voltage_protection_binary_sensor_ = voltage_protection_binary_sensor;
  }
  void set_temperature_protection_binary_sensor(binary_sensor::BinarySensor *temperature_protection_binary_sensor) {
    temperature_protection_binary_sensor_ = temperature_protection_binary_sensor;
  }
  void set_current_protection_binary_sensor(binary_sensor::BinarySensor *current_protection_binary_sensor) {
    current_protection_binary_sensor_ = current_protection_binary_sensor;
  }
  void set_soc_protection_binary_sensor(binary_sensor::BinarySensor *soc_protection_binary_sensor) {
    soc_protection_binary_sensor_ = soc_protection_binary_sensor;
  }
  void set_charging_binary_sensor(binary_sensor::BinarySensor *charging_binary_sensor) {
    charging_binary_sensor_ = charging_binary_sensor;
  }
  void set_discharging_binary_sensor(binary_sensor::BinarySensor *discharging_binary_sensor) {
    discharging_binary_sensor_ = discharging_binary_sensor;
  }
  void set_balancing_binary_sensor(binary_sensor::BinarySensor *balancing_binary_sensor) {
    balancing_binary_sensor_ = balancing_binary_sensor;
  }

  // Alarm event bitmask sensors
  void set_alarm_event1_bitmask_sensor(sensor::Sensor *sensor) { alarm_event1_bitmask_sensor_ = sensor; }
  void set_alarm_event2_bitmask_sensor(sensor::Sensor *sensor) { alarm_event2_bitmask_sensor_ = sensor; }
  void set_alarm_event3_bitmask_sensor(sensor::Sensor *sensor) { alarm_event3_bitmask_sensor_ = sensor; }
  void set_alarm_event4_bitmask_sensor(sensor::Sensor *sensor) { alarm_event4_bitmask_sensor_ = sensor; }
  void set_alarm_event5_bitmask_sensor(sensor::Sensor *sensor) { alarm_event5_bitmask_sensor_ = sensor; }
  void set_alarm_event6_bitmask_sensor(sensor::Sensor *sensor) { alarm_event6_bitmask_sensor_ = sensor; }
  void set_alarm_event7_bitmask_sensor(sensor::Sensor *sensor) { alarm_event7_bitmask_sensor_ = sensor; }
  void set_alarm_event8_bitmask_sensor(sensor::Sensor *sensor) { alarm_event8_bitmask_sensor_ = sensor; }
  void set_alarms_text_sensor(text_sensor::TextSensor *sensor) { alarms_text_sensor_ = sensor; }

  // Balancing sensors
  void set_balancing_bitmask_sensor(sensor::Sensor *sensor) { balancing_bitmask_sensor_ = sensor; }

  // Disconnection sensors
  void set_disconnection_bitmask_sensor(sensor::Sensor *sensor) { disconnection_bitmask_sensor_ = sensor; }

  void set_min_cell_voltage_sensor(sensor::Sensor *min_cell_voltage_sensor) {
    min_cell_voltage_sensor_ = min_cell_voltage_sensor;
  }
  void set_max_cell_voltage_sensor(sensor::Sensor *max_cell_voltage_sensor) {
    max_cell_voltage_sensor_ = max_cell_voltage_sensor;
  }
  void set_min_voltage_cell_sensor(sensor::Sensor *min_voltage_cell_sensor) {
    min_voltage_cell_sensor_ = min_voltage_cell_sensor;
  }
  void set_max_voltage_cell_sensor(sensor::Sensor *max_voltage_cell_sensor) {
    max_voltage_cell_sensor_ = max_voltage_cell_sensor;
  }
  void set_delta_cell_voltage_sensor(sensor::Sensor *delta_cell_voltage_sensor) {
    delta_cell_voltage_sensor_ = delta_cell_voltage_sensor;
  }
  void set_average_cell_voltage_sensor(sensor::Sensor *average_cell_voltage_sensor) {
    average_cell_voltage_sensor_ = average_cell_voltage_sensor;
  }
  void set_cell_voltage_sensor(uint8_t cell, sensor::Sensor *cell_voltage_sensor) {
    this->cells_[cell].cell_voltage_sensor_ = cell_voltage_sensor;
  }
  void set_temperature_sensor(uint8_t temperature, sensor::Sensor *temperature_sensor) {
    this->temperatures_[temperature].temperature_sensor_ = temperature_sensor;
  }
  void set_total_voltage_sensor(sensor::Sensor *total_voltage_sensor) { total_voltage_sensor_ = total_voltage_sensor; }
  void set_current_sensor(sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
  void set_charging_power_sensor(sensor::Sensor *charging_power_sensor) {
    charging_power_sensor_ = charging_power_sensor;
  }
  void set_discharging_power_sensor(sensor::Sensor *discharging_power_sensor) {
    discharging_power_sensor_ = discharging_power_sensor;
  }
  void set_state_of_charge_sensor(sensor::Sensor *state_of_charge_sensor) {
    state_of_charge_sensor_ = state_of_charge_sensor;
  }
  void set_residual_capacity_sensor(sensor::Sensor *residual_capacity_sensor) {
    residual_capacity_sensor_ = residual_capacity_sensor;
  }
  void set_battery_capacity_sensor(sensor::Sensor *battery_capacity_sensor) {
    battery_capacity_sensor_ = battery_capacity_sensor;
  }
  void set_rated_capacity_sensor(sensor::Sensor *rated_capacity_sensor) {
    rated_capacity_sensor_ = rated_capacity_sensor;
  }
  void set_charging_cycles_sensor(sensor::Sensor *charging_cycles_sensor) {
    charging_cycles_sensor_ = charging_cycles_sensor;
  }
  void set_state_of_health_sensor(sensor::Sensor *state_of_health_sensor) {
    state_of_health_sensor_ = state_of_health_sensor;
  }
  void set_port_voltage_sensor(sensor::Sensor *port_voltage_sensor) { port_voltage_sensor_ = port_voltage_sensor; }

  void set_errors_text_sensor(text_sensor::TextSensor *errors_text_sensor) { errors_text_sensor_ = errors_text_sensor; }

  void set_override_cell_count(uint8_t override_cell_count) { this->override_cell_count_ = override_cell_count; }

  void on_seplos_modbus_data(const std::vector<uint8_t> &data) override;

  void dump_config() override;
  void update() override;
  float get_setup_priority() const override;

 protected:
  binary_sensor::BinarySensor *online_status_binary_sensor_;
  binary_sensor::BinarySensor *voltage_protection_binary_sensor_;
  binary_sensor::BinarySensor *temperature_protection_binary_sensor_;
  binary_sensor::BinarySensor *current_protection_binary_sensor_;
  binary_sensor::BinarySensor *soc_protection_binary_sensor_;
  binary_sensor::BinarySensor *charging_binary_sensor_;
  binary_sensor::BinarySensor *discharging_binary_sensor_;
  binary_sensor::BinarySensor *balancing_binary_sensor_;

  // Alarm event bitmask sensors
  sensor::Sensor *alarm_event1_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event2_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event3_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event4_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event5_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event6_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event7_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event8_bitmask_sensor_{nullptr};
  text_sensor::TextSensor *alarms_text_sensor_{nullptr};

  // Balancing sensors
  sensor::Sensor *balancing_bitmask_sensor_{nullptr};

  // Disconnection sensors
  sensor::Sensor *disconnection_bitmask_sensor_{nullptr};

  sensor::Sensor *min_cell_voltage_sensor_;
  sensor::Sensor *max_cell_voltage_sensor_;
  sensor::Sensor *min_voltage_cell_sensor_;
  sensor::Sensor *max_voltage_cell_sensor_;
  sensor::Sensor *delta_cell_voltage_sensor_;
  sensor::Sensor *average_cell_voltage_sensor_;
  sensor::Sensor *total_voltage_sensor_;
  sensor::Sensor *current_sensor_;
  sensor::Sensor *power_sensor_;
  sensor::Sensor *charging_power_sensor_;
  sensor::Sensor *discharging_power_sensor_;
  sensor::Sensor *state_of_charge_sensor_;
  sensor::Sensor *residual_capacity_sensor_;
  sensor::Sensor *battery_capacity_sensor_;
  sensor::Sensor *rated_capacity_sensor_;
  sensor::Sensor *charging_cycles_sensor_;
  sensor::Sensor *state_of_health_sensor_;
  sensor::Sensor *port_voltage_sensor_;

  text_sensor::TextSensor *errors_text_sensor_;

  struct Cell {
    sensor::Sensor *cell_voltage_sensor_{nullptr};
  } cells_[16];

  struct Temperature {
    sensor::Sensor *temperature_sensor_{nullptr};
  } temperatures_[6];

  uint8_t override_cell_count_{0};
  uint8_t no_response_count_{0};

  void publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state);
  void publish_state_(sensor::Sensor *sensor, float value);
  void publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state);
  void on_telemetry_data_(const std::vector<uint8_t> &data);
  void on_alarm_data_(const std::vector<uint8_t> &data);
  void reset_online_status_tracker_();
  void track_online_status_();
  void publish_device_unavailable_();

  // Helper functions for alarm decoding
  std::string bitmask_to_string_(const char *const messages[], const uint8_t &messages_size, const uint8_t &mask);
  std::string decode_all_alarm_events_(uint8_t alarm_event1, uint8_t alarm_event2, uint8_t alarm_event3,
                                       uint8_t alarm_event4, uint8_t alarm_event5, uint8_t alarm_event6,
                                       uint8_t alarm_event7, uint8_t alarm_event8);
};

}  // namespace seplos_bms
}  // namespace esphome
