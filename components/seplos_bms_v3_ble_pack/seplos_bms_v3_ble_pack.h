#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "../seplos_bms_v3_ble/seplos_bms_v3_ble.h"

namespace esphome {
namespace seplos_bms_v3_ble_pack {

class SeplosBmsV3BlePack : public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_address(uint8_t address) { address_ = address; }
  void set_parent(seplos_bms_v3_ble::SeplosBmsV3Ble *parent) { parent_ = parent; }

  // Pack-spezifische Sensoren
  void set_pack_voltage_sensor(sensor::Sensor *sensor) { pack_voltage_sensor_ = sensor; }
  void set_pack_current_sensor(sensor::Sensor *sensor) { pack_current_sensor_ = sensor; }
  void set_pack_battery_level_sensor(sensor::Sensor *sensor) { pack_battery_level_sensor_ = sensor; }
  void set_pack_cycle_sensor(sensor::Sensor *sensor) { pack_cycle_sensor_ = sensor; }

  // Generic index-based setters
  void set_pack_cell_voltage_sensor(uint8_t index, sensor::Sensor *sensor) {
    if (index < 32)
      pack_cell_voltage_sensors_[index] = sensor;
  }
  void set_pack_temperature_sensor(uint8_t index, sensor::Sensor *sensor) {
    if (index < 16)
      pack_temperature_sensors_[index] = sensor;
  }

  // Additional pack-specific environment sensors (PIB registers 1118-1119)
  void set_ambient_temperature_sensor(sensor::Sensor *sensor) { ambient_temperature_sensor_ = sensor; }
  void set_mosfet_temperature_sensor(sensor::Sensor *sensor) { mosfet_temperature_sensor_ = sensor; }

  // Methods for data updates
  void update_pack_voltage(float voltage);
  void update_pack_current(float current);
  void update_pack_battery_level(float level);
  void update_pack_cycle(float cycle);
  void update_pack_cell_voltage(uint8_t index, float voltage);
  void update_pack_temperature(uint8_t index, float temperature);

  // Pack-specific data decoding methods
  void decode_pia_data(const std::vector<uint8_t> &data);
  void decode_pib_data(const std::vector<uint8_t> &data);
  void decode_pic_data(const std::vector<uint8_t> &data);

  uint8_t get_address() const { return address_; }

 protected:
  uint8_t address_;
  seplos_bms_v3_ble::SeplosBmsV3Ble *parent_;

  sensor::Sensor *pack_voltage_sensor_{nullptr};
  sensor::Sensor *pack_current_sensor_{nullptr};
  sensor::Sensor *pack_battery_level_sensor_{nullptr};
  sensor::Sensor *pack_cycle_sensor_{nullptr};
  sensor::Sensor *pack_cell_voltage_sensors_[32]{nullptr};
  sensor::Sensor *pack_temperature_sensors_[16]{nullptr};
  sensor::Sensor *ambient_temperature_sensor_{nullptr};
  sensor::Sensor *mosfet_temperature_sensor_{nullptr};
};

}  // namespace seplos_bms_v3_ble_pack
}  // namespace esphome
