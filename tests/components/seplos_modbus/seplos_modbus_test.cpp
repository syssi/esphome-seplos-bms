#include <gtest/gtest.h>
#include "common.h"

namespace esphome::seplos_modbus::testing {

TEST(SeplosModbusTest, ValidFrameDispatchedToDevice) {
  TestableSeplosModbus modbus;
  MockSeplosModbusDevice device;
  device.set_address(0x00);
  modbus.register_device(&device);

  modbus.feed(FRAME_ADDR_00);

  EXPECT_EQ(device.call_count, 1);
}

TEST(SeplosModbusTest, FrameDataDecodedCorrectly) {
  TestableSeplosModbus modbus;
  MockSeplosModbusDevice device;
  device.set_address(0x00);
  modbus.register_device(&device);

  modbus.feed(FRAME_ADDR_00);

  // ASCII "20004600" decodes to [0x20, 0x00, 0x46, 0x00]
  ASSERT_EQ(device.received_data.size(), 4u);
  EXPECT_EQ(device.received_data[0], 0x20);
  EXPECT_EQ(device.received_data[1], 0x00);
  EXPECT_EQ(device.received_data[2], 0x46);
  EXPECT_EQ(device.received_data[3], 0x00);
}

TEST(SeplosModbusTest, TwoFramesDispatchedTwice) {
  TestableSeplosModbus modbus;
  MockSeplosModbusDevice device;
  device.set_address(0x00);
  modbus.register_device(&device);

  modbus.feed(FRAME_ADDR_00);
  modbus.feed(FRAME_ADDR_00);

  EXPECT_EQ(device.call_count, 2);
}

TEST(SeplosModbusTest, BadChecksumRejected) {
  TestableSeplosModbus modbus;
  MockSeplosModbusDevice device;
  device.set_address(0x00);
  modbus.register_device(&device);

  std::vector<uint8_t> bad_frame = FRAME_ADDR_00;
  bad_frame[bad_frame.size() - 2] ^= 0xFF;  // corrupt CRC digit
  modbus.feed(bad_frame);

  EXPECT_EQ(device.call_count, 0);
}

TEST(SeplosModbusTest, UnknownAddressNotDispatched) {
  TestableSeplosModbus modbus;
  MockSeplosModbusDevice device;
  device.set_address(0x99);
  modbus.register_device(&device);

  modbus.feed(FRAME_ADDR_00);  // address=0x00 != 0x99

  EXPECT_EQ(device.call_count, 0);
}

TEST(SeplosModbusTest, NoRegisteredDeviceDoesNotCrash) {
  TestableSeplosModbus modbus;
  modbus.feed(FRAME_ADDR_00);
}

TEST(SeplosModbusTest, PartialFrameDoesNotDispatch) {
  TestableSeplosModbus modbus;
  MockSeplosModbusDevice device;
  device.set_address(0x00);
  modbus.register_device(&device);

  for (size_t i = 0; i < FRAME_ADDR_00.size() - 2; i++)
    modbus.parse_seplos_modbus_byte_(FRAME_ADDR_00[i]);

  EXPECT_EQ(device.call_count, 0);
}

TEST(SeplosModbusTest, InvalidHeaderRejected) {
  TestableSeplosModbus modbus;
  MockSeplosModbusDevice device;
  device.set_address(0x00);
  modbus.register_device(&device);

  bool result = modbus.parse_seplos_modbus_byte_(0x00);  // not 0x7E

  EXPECT_FALSE(result);
  EXPECT_EQ(device.call_count, 0);
}

TEST(SeplosModbusTest, AddressMatchSelectsCorrectDevice) {
  TestableSeplosModbus modbus;
  MockSeplosModbusDevice device_00, device_01;
  device_00.set_address(0x00);
  device_01.set_address(0x01);
  modbus.register_device(&device_00);
  modbus.register_device(&device_01);

  modbus.feed(FRAME_ADDR_01);

  EXPECT_EQ(device_00.call_count, 0);
  EXPECT_EQ(device_01.call_count, 1);
}

}  // namespace esphome::seplos_modbus::testing
