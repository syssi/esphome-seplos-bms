#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

#ifdef USE_ESP32

#include <esp_gattc_api.h>

namespace esphome {

namespace seplos_bms_v3_ble_pack {
class SeplosBmsV3BlePack;
}

namespace seplos_bms_v3_ble {

namespace espbt = esphome::esp32_ble_tracker;

class SeplosBmsV3Ble;

class SeplosBmsV3BlePack {
 public:
  void set_parent(SeplosBmsV3Ble *parent) { parent_ = parent; }
  void set_address(uint8_t address) { address_ = address; }
  virtual void on_frame_data(const std::vector<uint8_t> &frame) = 0;
  uint8_t get_address() const { return address_; }

 protected:
  friend SeplosBmsV3Ble;
  SeplosBmsV3Ble *parent_;
  uint8_t address_;
};

struct SeplosV3Command {
  uint8_t device;
  uint8_t function;
  uint16_t reg_start;
  uint16_t reg_count;
};

class SeplosBmsV3Ble : public esphome::ble_client::BLEClientNode, public PollingComponent {
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
  void set_system_fault_binary_sensor(binary_sensor::BinarySensor *system_fault_binary_sensor) {
    system_fault_binary_sensor_ = system_fault_binary_sensor;
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
  void set_charging_cycles_sensor(sensor::Sensor *charging_cycles_sensor) {
    charging_cycles_sensor_ = charging_cycles_sensor;
  }
  void set_average_cell_temperature_sensor(sensor::Sensor *average_cell_temperature_sensor) {
    average_cell_temperature_sensor_ = average_cell_temperature_sensor;
  }
  void set_pack_count_sensor(sensor::Sensor *pack_count_sensor) { pack_count_sensor_ = pack_count_sensor; }
  void set_delta_voltage_sensor(sensor::Sensor *delta_voltage_sensor) { delta_voltage_sensor_ = delta_voltage_sensor; }
  void set_problem_code_sensor(sensor::Sensor *problem_code_sensor) { problem_code_sensor_ = problem_code_sensor; }
  void set_cycle_charge_sensor(sensor::Sensor *cycle_charge_sensor) { cycle_charge_sensor_ = cycle_charge_sensor; }
  void set_cycle_capacity_sensor(sensor::Sensor *cycle_capacity_sensor) {
    cycle_capacity_sensor_ = cycle_capacity_sensor;
  }
  void set_runtime_sensor(sensor::Sensor *runtime_sensor) { runtime_sensor_ = runtime_sensor; }
  void set_state_of_health_sensor(sensor::Sensor *state_of_health_sensor) {
    state_of_health_sensor_ = state_of_health_sensor;
  }
  void set_capacity_remaining_sensor(sensor::Sensor *capacity_remaining_sensor) {
    capacity_remaining_sensor_ = capacity_remaining_sensor;
  }
  void set_total_capacity_sensor(sensor::Sensor *total_capacity_sensor) {
    total_capacity_sensor_ = total_capacity_sensor;
  }
  void set_rated_capacity_sensor(sensor::Sensor *rated_capacity_sensor) {
    rated_capacity_sensor_ = rated_capacity_sensor;
  }
  void set_ambient_temperature_sensor(sensor::Sensor *ambient_temperature_sensor) {
    ambient_temperature_sensor_ = ambient_temperature_sensor;
  }
  void set_mosfet_temperature_sensor(sensor::Sensor *mosfet_temperature_sensor) {
    mosfet_temperature_sensor_ = mosfet_temperature_sensor;
  }
  void set_min_cell_temperature_sensor(sensor::Sensor *min_cell_temperature_sensor) {
    min_cell_temperature_sensor_ = min_cell_temperature_sensor;
  }
  void set_max_cell_temperature_sensor(sensor::Sensor *max_cell_temperature_sensor) {
    max_cell_temperature_sensor_ = max_cell_temperature_sensor;
  }
  void set_min_temperature_cell_sensor(sensor::Sensor *min_temperature_cell_sensor) {
    min_temperature_cell_sensor_ = min_temperature_cell_sensor;
  }
  void set_max_temperature_cell_sensor(sensor::Sensor *max_temperature_cell_sensor) {
    max_temperature_cell_sensor_ = max_temperature_cell_sensor;
  }
  void set_min_pack_voltage_sensor(sensor::Sensor *min_pack_voltage_sensor) {
    min_pack_voltage_sensor_ = min_pack_voltage_sensor;
  }
  void set_max_pack_voltage_sensor(sensor::Sensor *max_pack_voltage_sensor) {
    max_pack_voltage_sensor_ = max_pack_voltage_sensor;
  }
  void set_min_pack_voltage_id_sensor(sensor::Sensor *min_pack_voltage_id_sensor) {
    min_pack_voltage_id_sensor_ = min_pack_voltage_id_sensor;
  }
  void set_max_pack_voltage_id_sensor(sensor::Sensor *max_pack_voltage_id_sensor) {
    max_pack_voltage_id_sensor_ = max_pack_voltage_id_sensor;
  }
  void set_system_state_code_sensor(sensor::Sensor *system_state_code_sensor) {
    system_state_code_sensor_ = system_state_code_sensor;
  }
  void set_voltage_event_code_sensor(sensor::Sensor *voltage_event_code_sensor) {
    voltage_event_code_sensor_ = voltage_event_code_sensor;
  }
  void set_temperature_event_code_sensor(sensor::Sensor *temperature_event_code_sensor) {
    temperature_event_code_sensor_ = temperature_event_code_sensor;
  }
  void set_current_event_code_sensor(sensor::Sensor *current_event_code_sensor) {
    current_event_code_sensor_ = current_event_code_sensor;
  }
  void set_max_discharge_current_sensor(sensor::Sensor *max_discharge_current_sensor) {
    max_discharge_current_sensor_ = max_discharge_current_sensor;
  }
  void set_max_charge_current_sensor(sensor::Sensor *max_charge_current_sensor) {
    max_charge_current_sensor_ = max_charge_current_sensor;
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

  void set_problem_text_sensor(text_sensor::TextSensor *problem_text_sensor) {
    problem_text_sensor_ = problem_text_sensor;
  }
  void set_factory_name_text_sensor(text_sensor::TextSensor *factory_name_text_sensor) {
    factory_name_text_sensor_ = factory_name_text_sensor;
  }
  void set_device_name_text_sensor(text_sensor::TextSensor *device_name_text_sensor) {
    device_name_text_sensor_ = device_name_text_sensor;
  }
  void set_firmware_version_text_sensor(text_sensor::TextSensor *firmware_version_text_sensor) {
    firmware_version_text_sensor_ = firmware_version_text_sensor;
  }
  void set_bms_serial_number_text_sensor(text_sensor::TextSensor *bms_serial_number_text_sensor) {
    bms_serial_number_text_sensor_ = bms_serial_number_text_sensor;
  }
  void set_pack_serial_number_text_sensor(text_sensor::TextSensor *pack_serial_number_text_sensor) {
    pack_serial_number_text_sensor_ = pack_serial_number_text_sensor;
  }

  void register_pack_component(SeplosBmsV3BlePack *pack_device) {
    pack_devices_.push_back(pack_device);
    // Note: Command queue will be built during setup/connection to include commands for registered packs
  }

  void assemble(const uint8_t *data, uint16_t length);
  void decode(const std::vector<uint8_t> &data);

 protected:
  binary_sensor::BinarySensor *charging_binary_sensor_;
  binary_sensor::BinarySensor *discharging_binary_sensor_;
  binary_sensor::BinarySensor *online_status_binary_sensor_;
  binary_sensor::BinarySensor *voltage_protection_binary_sensor_;
  binary_sensor::BinarySensor *temperature_protection_binary_sensor_;
  binary_sensor::BinarySensor *current_protection_binary_sensor_;
  binary_sensor::BinarySensor *system_fault_binary_sensor_;

  sensor::Sensor *total_voltage_sensor_;
  sensor::Sensor *current_sensor_;
  sensor::Sensor *power_sensor_;
  sensor::Sensor *charging_power_sensor_;
  sensor::Sensor *discharging_power_sensor_;
  sensor::Sensor *state_of_charge_sensor_;
  sensor::Sensor *charging_cycles_sensor_;
  sensor::Sensor *average_cell_temperature_sensor_;
  sensor::Sensor *pack_count_sensor_;
  sensor::Sensor *delta_voltage_sensor_;
  sensor::Sensor *problem_code_sensor_;
  sensor::Sensor *cycle_charge_sensor_;
  sensor::Sensor *cycle_capacity_sensor_;
  sensor::Sensor *runtime_sensor_;
  sensor::Sensor *state_of_health_sensor_;
  sensor::Sensor *capacity_remaining_sensor_;
  sensor::Sensor *total_capacity_sensor_;
  sensor::Sensor *rated_capacity_sensor_;
  sensor::Sensor *ambient_temperature_sensor_;
  sensor::Sensor *mosfet_temperature_sensor_;
  sensor::Sensor *min_cell_temperature_sensor_;
  sensor::Sensor *max_cell_temperature_sensor_;
  sensor::Sensor *min_temperature_cell_sensor_;
  sensor::Sensor *max_temperature_cell_sensor_;
  sensor::Sensor *min_pack_voltage_sensor_;
  sensor::Sensor *max_pack_voltage_sensor_;
  sensor::Sensor *min_pack_voltage_id_sensor_;
  sensor::Sensor *max_pack_voltage_id_sensor_;
  sensor::Sensor *system_state_code_sensor_;
  sensor::Sensor *voltage_event_code_sensor_;
  sensor::Sensor *temperature_event_code_sensor_;
  sensor::Sensor *current_event_code_sensor_;
  sensor::Sensor *max_discharge_current_sensor_;
  sensor::Sensor *max_charge_current_sensor_;

  sensor::Sensor *min_cell_voltage_sensor_;
  sensor::Sensor *max_cell_voltage_sensor_;
  sensor::Sensor *min_voltage_cell_sensor_;
  sensor::Sensor *max_voltage_cell_sensor_;

  text_sensor::TextSensor *problem_text_sensor_;
  text_sensor::TextSensor *factory_name_text_sensor_;
  text_sensor::TextSensor *device_name_text_sensor_;
  text_sensor::TextSensor *firmware_version_text_sensor_;
  text_sensor::TextSensor *bms_serial_number_text_sensor_;
  text_sensor::TextSensor *pack_serial_number_text_sensor_;

  std::vector<uint8_t> frame_buffer_;
  uint16_t char_notify_handle_;
  uint16_t char_command_handle_;
  uint8_t next_command_{0};
  uint8_t pack_count_{0};
  std::vector<SeplosBmsV3BlePack *> pack_devices_;
  std::vector<SeplosV3Command> dynamic_command_queue_;
  std::vector<uint8_t> build_modbus_payload_(const SeplosV3Command &cmd);
  uint16_t crc16_(const uint8_t *data, uint16_t length);

  void publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state);
  void publish_state_(sensor::Sensor *sensor, float value);
  void publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state);
  bool send_command_(uint8_t function, const std::vector<uint8_t> &payload);
  void decode_eia_data_(const std::vector<uint8_t> &data);
  void decode_eib_data_(const std::vector<uint8_t> &data);
  void decode_eic_data_(const std::vector<uint8_t> &data);
  void decode_pct_data_(const std::vector<uint8_t> &data);
  void decode_sfa_data_(const std::vector<uint8_t> &data);
  void decode_spa_data_(const std::vector<uint8_t> &data);
  void build_dynamic_command_queue_();
};

}  // namespace seplos_bms_v3_ble
}  // namespace esphome

#endif
