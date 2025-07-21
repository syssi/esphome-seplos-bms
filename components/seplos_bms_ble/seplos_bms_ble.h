#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/switch/switch.h"

#ifdef USE_ESP32

#include <esp_gattc_api.h>

namespace esphome {
namespace seplos_bms_ble {

namespace espbt = esphome::esp32_ble_tracker;

class SeplosBmsBle : public esphome::ble_client::BLEClientNode, public PollingComponent {
 public:
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;
  void dump_config() override;
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_charging_binary_sensor(binary_sensor::BinarySensor *charging_binary_sensor) {
    charging_binary_sensor_ = charging_binary_sensor;
  }
  void set_discharging_binary_sensor(binary_sensor::BinarySensor *discharging_binary_sensor) {
    discharging_binary_sensor_ = discharging_binary_sensor;
  }
  void set_limiting_current_binary_sensor(binary_sensor::BinarySensor *limiting_current_binary_sensor) {
    limiting_current_binary_sensor_ = limiting_current_binary_sensor;
  }
  void set_online_status_binary_sensor(binary_sensor::BinarySensor *online_status_binary_sensor) {
    online_status_binary_sensor_ = online_status_binary_sensor;
  }

  // Enhanced alarm bitmask and consolidated alarms text sensor
  void set_alarm_event1_bitmask_sensor(sensor::Sensor *sensor) { alarm_event1_bitmask_sensor_ = sensor; }
  void set_alarm_event2_bitmask_sensor(sensor::Sensor *sensor) { alarm_event2_bitmask_sensor_ = sensor; }
  void set_alarm_event3_bitmask_sensor(sensor::Sensor *sensor) { alarm_event3_bitmask_sensor_ = sensor; }
  void set_alarm_event4_bitmask_sensor(sensor::Sensor *sensor) { alarm_event4_bitmask_sensor_ = sensor; }
  void set_alarm_event5_bitmask_sensor(sensor::Sensor *sensor) { alarm_event5_bitmask_sensor_ = sensor; }
  void set_alarm_event6_bitmask_sensor(sensor::Sensor *sensor) { alarm_event6_bitmask_sensor_ = sensor; }
  void set_alarm_event7_bitmask_sensor(sensor::Sensor *sensor) { alarm_event7_bitmask_sensor_ = sensor; }
  void set_alarm_event8_bitmask_sensor(sensor::Sensor *sensor) { alarm_event8_bitmask_sensor_ = sensor; }
  void set_alarms_text_sensor(text_sensor::TextSensor *sensor) { alarms_text_sensor_ = sensor; }

  void set_total_voltage_sensor(sensor::Sensor *total_voltage_sensor) { total_voltage_sensor_ = total_voltage_sensor; }
  void set_current_sensor(sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
  void set_charging_power_sensor(sensor::Sensor *charging_power_sensor) {
    charging_power_sensor_ = charging_power_sensor;
  }
  void set_discharging_power_sensor(sensor::Sensor *discharging_power_sensor) {
    discharging_power_sensor_ = discharging_power_sensor;
  }
  void set_capacity_remaining_sensor(sensor::Sensor *capacity_remaining_sensor) {
    capacity_remaining_sensor_ = capacity_remaining_sensor;
  }

  void set_voltage_protection_bitmask_sensor(sensor::Sensor *voltage_protection_bitmask_sensor) {
    voltage_protection_bitmask_sensor_ = voltage_protection_bitmask_sensor;
  }
  void set_current_protection_bitmask_sensor(sensor::Sensor *current_protection_bitmask_sensor) {
    current_protection_bitmask_sensor_ = current_protection_bitmask_sensor;
  }
  void set_temperature_protection_bitmask_sensor(sensor::Sensor *temperature_protection_bitmask_sensor) {
    temperature_protection_bitmask_sensor_ = temperature_protection_bitmask_sensor;
  }
  void set_state_of_charge_sensor(sensor::Sensor *state_of_charge_sensor) {
    state_of_charge_sensor_ = state_of_charge_sensor;
  }
  void set_nominal_capacity_sensor(sensor::Sensor *nominal_capacity_sensor) {
    nominal_capacity_sensor_ = nominal_capacity_sensor;
  }
  void set_charging_cycles_sensor(sensor::Sensor *charging_cycles_sensor) {
    charging_cycles_sensor_ = charging_cycles_sensor;
  }
  void set_average_cell_temperature_sensor(sensor::Sensor *average_cell_temperature_sensor) {
    average_cell_temperature_sensor_ = average_cell_temperature_sensor;
  }
  void set_ambient_temperature_sensor(sensor::Sensor *ambient_temperature_sensor) {
    ambient_temperature_sensor_ = ambient_temperature_sensor;
  }
  void set_mosfet_temperature_sensor(sensor::Sensor *mosfet_temperature_sensor) {
    mosfet_temperature_sensor_ = mosfet_temperature_sensor;
  }
  void set_state_of_health_sensor(sensor::Sensor *state_of_health_sensor) {
    state_of_health_sensor_ = state_of_health_sensor;
  }
  void set_port_voltage_sensor(sensor::Sensor *port_voltage_sensor) { port_voltage_sensor_ = port_voltage_sensor; }
  void set_battery_capacity_sensor(sensor::Sensor *battery_capacity_sensor) {
    battery_capacity_sensor_ = battery_capacity_sensor;
  }

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

  void set_software_version_text_sensor(text_sensor::TextSensor *software_version_text_sensor) {
    software_version_text_sensor_ = software_version_text_sensor;
  }
  void set_device_model_text_sensor(text_sensor::TextSensor *device_model_text_sensor) {
    device_model_text_sensor_ = device_model_text_sensor;
  }
  void set_hardware_version_text_sensor(text_sensor::TextSensor *hardware_version_text_sensor) {
    hardware_version_text_sensor_ = hardware_version_text_sensor;
  }
  void set_voltage_protection_text_sensor(text_sensor::TextSensor *voltage_protection_text_sensor) {
    voltage_protection_text_sensor_ = voltage_protection_text_sensor;
  }
  void set_current_protection_text_sensor(text_sensor::TextSensor *current_protection_text_sensor) {
    current_protection_text_sensor_ = current_protection_text_sensor;
  }
  void set_temperature_protection_text_sensor(text_sensor::TextSensor *temperature_protection_text_sensor) {
    temperature_protection_text_sensor_ = temperature_protection_text_sensor;
  }

  void set_discharging_switch(switch_::Switch *discharging_switch) { discharging_switch_ = discharging_switch; }
  void set_charging_switch(switch_::Switch *charging_switch) { charging_switch_ = charging_switch; }
  void set_current_limit_switch(switch_::Switch *current_limit_switch) { current_limit_switch_ = current_limit_switch; }
  void set_heating_switch(switch_::Switch *heating_switch) { heating_switch_ = heating_switch; }

  bool send_command(uint8_t function, const std::vector<uint8_t> &payload = {});
  void assemble(const uint8_t *data, uint16_t length);
  void decode(const std::vector<uint8_t> &data);
  std::string interpret_can_protocol(uint8_t value);
  std::string interpret_rs485_protocol(uint8_t value);
  std::string interpret_battery_type(uint8_t value);

 protected:
  binary_sensor::BinarySensor *charging_binary_sensor_;
  binary_sensor::BinarySensor *discharging_binary_sensor_;
  binary_sensor::BinarySensor *limiting_current_binary_sensor_;
  binary_sensor::BinarySensor *online_status_binary_sensor_;

  // Enhanced alarm bitmask and consolidated alarms text sensor
  sensor::Sensor *alarm_event1_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event2_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event3_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event4_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event5_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event6_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event7_bitmask_sensor_{nullptr};
  sensor::Sensor *alarm_event8_bitmask_sensor_{nullptr};
  text_sensor::TextSensor *alarms_text_sensor_{nullptr};

  sensor::Sensor *total_voltage_sensor_;
  sensor::Sensor *current_sensor_;
  sensor::Sensor *power_sensor_;
  sensor::Sensor *charging_power_sensor_;
  sensor::Sensor *discharging_power_sensor_;
  sensor::Sensor *capacity_remaining_sensor_;
  sensor::Sensor *voltage_protection_bitmask_sensor_;
  sensor::Sensor *current_protection_bitmask_sensor_;
  sensor::Sensor *temperature_protection_bitmask_sensor_;
  sensor::Sensor *state_of_charge_sensor_;
  sensor::Sensor *nominal_capacity_sensor_;
  sensor::Sensor *charging_cycles_sensor_;
  sensor::Sensor *min_cell_voltage_sensor_;
  sensor::Sensor *max_cell_voltage_sensor_;
  sensor::Sensor *min_voltage_cell_sensor_;
  sensor::Sensor *max_voltage_cell_sensor_;
  sensor::Sensor *delta_cell_voltage_sensor_;
  sensor::Sensor *average_cell_voltage_sensor_;
  sensor::Sensor *average_cell_temperature_sensor_;
  sensor::Sensor *ambient_temperature_sensor_;
  sensor::Sensor *mosfet_temperature_sensor_;
  sensor::Sensor *state_of_health_sensor_;
  sensor::Sensor *port_voltage_sensor_;
  sensor::Sensor *battery_capacity_sensor_;

  text_sensor::TextSensor *software_version_text_sensor_;
  text_sensor::TextSensor *device_model_text_sensor_;
  text_sensor::TextSensor *hardware_version_text_sensor_;
  text_sensor::TextSensor *voltage_protection_text_sensor_;
  text_sensor::TextSensor *current_protection_text_sensor_;
  text_sensor::TextSensor *temperature_protection_text_sensor_;

  switch_::Switch *discharging_switch_;
  switch_::Switch *charging_switch_;
  switch_::Switch *current_limit_switch_;
  switch_::Switch *heating_switch_;

  struct Cell {
    sensor::Sensor *cell_voltage_sensor_{nullptr};
  } cells_[24];

  struct Temperature {
    sensor::Sensor *temperature_sensor_{nullptr};
  } temperatures_[8];

  std::vector<uint8_t> frame_buffer_;
  uint16_t char_notify_handle_;
  uint16_t char_command_handle_;
  uint8_t next_command_{0};

  float min_cell_voltage_{100.0f};
  float max_cell_voltage_{-100.0f};
  uint8_t max_voltage_cell_{0};
  uint8_t min_voltage_cell_{0};

  void decode_manufacturer_info_data_(const std::vector<uint8_t> &data);
  void decode_single_machine_data_(const std::vector<uint8_t> &data);
  void decode_settings_data_(const std::vector<uint8_t> &data);
  void decode_parallel_data_(const std::vector<uint8_t> &data);
  void publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state);
  void publish_state_(sensor::Sensor *sensor, float value);
  void publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state);
  void publish_state_(switch_::Switch *obj, const bool &state);
  std::string bitmask_to_string_(const char *const messages[], const uint8_t &messages_size, const uint8_t &mask);
  std::string decode_all_alarm_events_(uint8_t alarm_event1, uint8_t alarm_event2, uint8_t alarm_event3,
                                       uint8_t alarm_event4, uint8_t alarm_event5, uint8_t alarm_event6,
                                       uint8_t alarm_event7, uint8_t alarm_event8);

  bool check_bit_(uint16_t mask, uint16_t flag) { return (mask & flag) == flag; }
};

}  // namespace seplos_bms_ble
}  // namespace esphome

#endif
