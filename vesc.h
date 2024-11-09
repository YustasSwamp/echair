#include <ESP32-TWAI-CAN.hpp>

// Default for ESP32
#define CAN_TX		5
#define CAN_RX		4

#define CAN_PACKET_SET_DUTY 0
#define CAN_PACKET_SET_CURRENT_BRAKE 2

#define CMD(cmd, id) (((cmd & 0xff) << 8) | (id & 0xff))

CanFrame rxFrame;

void vesc_setup() {
  if(ESP32Can.begin(ESP32Can.convertSpeed(500), CAN_TX, CAN_RX, 10, 10)) {
    Serial.println("CAN bus started!");
  } else {
    Serial.println("CAN bus failed!");
  }
}

void vesc_set_duty(float duty, uint32_t id) {
  CanFrame obdFrame = { 0 };
  uint32_t set_value = duty * 100000;

	obdFrame.identifier = CMD(CAN_PACKET_SET_DUTY, id);
	obdFrame.extd = 1;
	obdFrame.data_length_code = 4;
	obdFrame.data[0] = (set_value >> 24) & 0xff;
	obdFrame.data[1] = (set_value >> 16) & 0xff;
	obdFrame.data[2] = (set_value >> 8) & 0xff;
	obdFrame.data[3] = set_value & 0xff;
  ESP32Can.writeFrame(obdFrame);  // timeout defaults to 1 ms
}

void vesc_set_current_brake(float current, uint32_t id) {
  CanFrame obdFrame = { 0 };
  uint32_t set_value = current * 1000;

	obdFrame.identifier = CMD(CAN_PACKET_SET_CURRENT_BRAKE, id);
	obdFrame.extd = 1;
	obdFrame.data_length_code = 4;
	obdFrame.data[0] = (set_value >> 24) & 0xff;
	obdFrame.data[1] = (set_value >> 16) & 0xff;
	obdFrame.data[2] = (set_value >> 8) & 0xff;
	obdFrame.data[3] = set_value & 0xff;
  ESP32Can.writeFrame(obdFrame);  // timeout defaults to 1 ms
}