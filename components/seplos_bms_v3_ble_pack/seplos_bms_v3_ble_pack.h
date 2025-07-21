#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "../seplos_bms_v3_ble/seplos_bms_v3_ble.h"

#ifdef USE_ESP32

namespace esphome {
namespace seplos_bms_v3_ble_pack {

class SeplosBmsV3BlePack : public Component, public seplos_bms_v3_ble::SeplosBmsV3BlePack {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_pack_voltage_sensor(sensor::Sensor *sensor) { pack_voltage_sensor_ = sensor; }
  void set_pack_current_sensor(sensor::Sensor *sensor) { pack_current_sensor_ = sensor; }
  void set_pack_battery_level_sensor(sensor::Sensor *sensor) { pack_battery_level_sensor_ = sensor; }
  void set_pack_cycle_sensor(sensor::Sensor *sensor) { pack_cycle_sensor_ = sensor; }

  void set_pack_cell_voltage_sensor(uint8_t index, sensor::Sensor *sensor) {
    pack_cell_voltage_sensors_[index] = sensor;
  }
  void set_pack_temperature_sensor(uint8_t index, sensor::Sensor *sensor) { pack_temperature_sensors_[index] = sensor; }

  void set_ambient_temperature_sensor(sensor::Sensor *sensor) { ambient_temperature_sensor_ = sensor; }
  void set_mosfet_temperature_sensor(sensor::Sensor *sensor) { mosfet_temperature_sensor_ = sensor; }

  void on_frame_data(const std::vector<uint8_t> &frame) override;

 protected:
  void decode_pack_pia_data_(const std::vector<uint8_t> &data);
  void decode_pack_pib_data_(const std::vector<uint8_t> &data);
  void decode_pack_pic_data_(const std::vector<uint8_t> &data);
  void publish_state_(sensor::Sensor *sensor, float value);

  sensor::Sensor *pack_voltage_sensor_{nullptr};
  sensor::Sensor *pack_current_sensor_{nullptr};
  sensor::Sensor *pack_battery_level_sensor_{nullptr};
  sensor::Sensor *pack_cycle_sensor_{nullptr};
  sensor::Sensor *pack_cell_voltage_sensors_[16]{nullptr};
  sensor::Sensor *pack_temperature_sensors_[4]{nullptr};
  sensor::Sensor *ambient_temperature_sensor_{nullptr};
  sensor::Sensor *mosfet_temperature_sensor_{nullptr};
};

}  // namespace seplos_bms_v3_ble_pack
}  // namespace esphome

#endif
