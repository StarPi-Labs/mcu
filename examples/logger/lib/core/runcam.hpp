#pragma once

// This entire thing is AI-generated, I couldn't be arsed
// https://support.runcam.com/hc/en-us/articles/360014537794-RunCam-Device-Protocol

#include <Arduino.h>

// RunCam Protocol Constants
#define RCDEVICE_PROTOCOL_HEADER                           0xCC

// Command IDs
#define RCDEVICE_PROTOCOL_COMMAND_GET_DEVICE_INFO          0x00
#define RCDEVICE_PROTOCOL_COMMAND_CAMERA_CONTROL           0x01
#define RCDEVICE_PROTOCOL_COMMAND_5KEY_SIMULATION_PRESS    0x02
#define RCDEVICE_PROTOCOL_COMMAND_5KEY_SIMULATION_RELEASE  0x03
#define RCDEVICE_PROTOCOL_COMMAND_5KEY_CONNECTION          0x04
#define RCDEVICE_PROTOCOL_COMMAND_REQUEST_FC_ATTITUDE      0x50

// Display Commands
#define RCDEVICE_PROTOCOL_COMMAND_DISP_FILL_REGION         0x20
#define RCDEVICE_PROTOCOL_COMMAND_DISP_WRITE_CHAR          0x21
#define RCDEVICE_PROTOCOL_COMMAND_DISP_WRITE_HORT_STRING   0x22
#define RCDEVICE_PROTOCOL_COMMAND_DISP_WRITE_VERT_STRING   0x23
#define RCDEVICE_PROTOCOL_COMMAND_DISP_WRITE_CHARS         0x24

// Camera Control Action IDs
#define RCDEVICE_PROTOCOL_SIMULATE_WIFI_BTN                0x00
#define RCDEVICE_PROTOCOL_SIMULATE_POWER_BTN               0x01
#define RCDEVICE_PROTOCOL_CHANGE_MODE                      0x02
#define RCDEVICE_PROTOCOL_CHANGE_START_RECORDING           0x03
#define RCDEVICE_PROTOCOL_CHANGE_STOP_RECORDING            0x04

// 5-Key Press Simulation Action IDs
#define RCDEVICE_PROTOCOL_5KEY_SIMULATION_SET              0x01
#define RCDEVICE_PROTOCOL_5KEY_SIMULATION_LEFT             0x02
#define RCDEVICE_PROTOCOL_5KEY_SIMULATION_RIGHT            0x03
#define RCDEVICE_PROTOCOL_5KEY_SIMULATION_UP               0x04
#define RCDEVICE_PROTOCOL_5KEY_SIMULATION_DOWN             0x05

// 5-Key Connection Action IDs
#define RCDEVICE_PROTOCOL_5KEY_FUNCTION_OPEN               0x01
#define RCDEVICE_PROTOCOL_5KEY_FUNCTION_CLOSE              0x02

// Device Features
#define RCDEVICE_PROTOCOL_FEATURE_SIMULATE_POWER_BUTTON    (1 << 0)
#define RCDEVICE_PROTOCOL_FEATURE_SIMULATE_WIFI_BUTTON     (1 << 1)
#define RCDEVICE_PROTOCOL_FEATURE_CHANGE_MODE              (1 << 2)
#define RCDEVICE_PROTOCOL_FEATURE_SIMULATE_5_KEY_OSD_CABLE (1 << 3)
#define RCDEVICE_PROTOCOL_FEATURE_DEVICE_SETTINGS_ACCESS   (1 << 4)
#define RCDEVICE_PROTOCOL_FEATURE_DISPLAYP_PORT            (1 << 5)
#define RCDEVICE_PROTOCOL_FEATURE_START_RECORDING          (1 << 6)
#define RCDEVICE_PROTOCOL_FEATURE_STOP_RECORDING           (1 << 7)
#define RCDEVICE_PROTOCOL_FEATURE_FC_ATTITUDE              (1 << 9)

// Structure to parse Device Information
struct RunCamDeviceInfo {
	uint8_t protocolVersion;
	uint16_t features;
};

// Structure to receive Flight Controller Attitude
struct RunCamAttitude {
	uint16_t roll;
	uint16_t pitch;
	uint16_t yaw;
};

class RunCam {
private:
	Stream* _serial;

	// CRC8 checksum algorithm from RunCam Specification
	uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a) {
		crc ^= a;
		for (int ii = 0; ii < 8; ++ii) {
			if (crc & 0x80) {
				crc = (crc << 1) ^ 0xD5;
			} else {
				crc = crc << 1;
			}
		}
		return crc;
	}

	// Calculates CRC for a full data payload
	uint8_t calculateCRC(const uint8_t* data, size_t len) {
		uint8_t crc = 0;
		for (size_t i = 0; i < len; i++) {
			crc = crc8_dvb_s2(crc, data[i]);
		}
		return crc;
	}

	// Standard non-blocking packet reader with timeout
	bool readPacket(uint8_t* buffer, size_t expectedLen, uint32_t timeoutMs) {
		uint32_t startTime = millis();
		size_t index = 0;

		while (index < expectedLen && (millis() - startTime < timeoutMs)) {
			if (_serial->available()) {
				uint8_t b = _serial->read();
				if (index == 0 && b != RCDEVICE_PROTOCOL_HEADER) {
					continue; // Synchronize on Header (0xCC)
				}
				buffer[index++] = b;
			}
		}

		if (index != expectedLen) return false;

		// Verify CRC8 checksum
		uint8_t calcCrc = calculateCRC(buffer, expectedLen - 1);
		return calcCrc == buffer[expectedLen - 1];
	}

public:
	RunCam(Stream& serialPort) {
		_serial = &serialPort;
	}

	// 1. Read camera information
	bool getDeviceInfo(RunCamDeviceInfo& info, uint32_t timeout = 150) {
		uint8_t packet[3] = {
			RCDEVICE_PROTOCOL_HEADER,
			RCDEVICE_PROTOCOL_COMMAND_GET_DEVICE_INFO,
			0x00 // CRC placeholder
		};
		packet[2] = calculateCRC(packet, 2);
		_serial->write(packet, 3);

		uint8_t response[5];
		if (readPacket(response, 5, timeout)) {
			info.protocolVersion = response[1];
			info.features = ((uint16_t)response[3] << 8) | response[2];  // Little-endian Feature
			return true;
		}
		return false;
	}

	// 2. Camera control (WiFi Button, Power Button, Modes, Recording)
	void controlCamera(uint8_t actionID) {
		uint8_t packet[4] = {
			RCDEVICE_PROTOCOL_HEADER,
			RCDEVICE_PROTOCOL_COMMAND_CAMERA_CONTROL,
			actionID,
			0x00 // CRC placeholder
		};
		packet[3] = calculateCRC(packet, 3);
		_serial->write(packet, 4);
	}

	// 3. Simulate Press command of the 5-key remote control
	bool press5Key(uint8_t actionID, uint32_t timeout = 100) {
		uint8_t packet[4] = {
			RCDEVICE_PROTOCOL_HEADER,
			RCDEVICE_PROTOCOL_COMMAND_5KEY_SIMULATION_PRESS,
			actionID,
			0x00 // CRC placeholder
		};
		packet[3] = calculateCRC(packet, 3);
		_serial->write(packet, 4);

		uint8_t response[2];
		return readPacket(response, 2, timeout);  // Expects [0xCC, CRC]
	}

	// 4. Simulate Release command of the 5-key remote control
	bool release5Key(uint32_t timeout = 100) {
		uint8_t packet[3] = {
			RCDEVICE_PROTOCOL_HEADER,
			RCDEVICE_PROTOCOL_COMMAND_5KEY_SIMULATION_RELEASE,
			0x00 // CRC placeholder
		};
		packet[2] = calculateCRC(packet, 2);
		_serial->write(packet, 3);

		uint8_t response[2];
		return readPacket(response, 2, timeout);  // Expects [0xCC, CRC]
	}

	// 5. Simulate handshake/disconnection command
	bool connection5Key(uint8_t actionID, uint8_t& result, uint32_t timeout = 150) {
		uint8_t packet[4] = {
			RCDEVICE_PROTOCOL_HEADER,
			RCDEVICE_PROTOCOL_COMMAND_5KEY_CONNECTION,
			actionID,
			0x00 // CRC placeholder
		};
		packet[3] = calculateCRC(packet, 3);
		_serial->write(packet, 4);

		uint8_t response[3];
		if (readPacket(response, 3, timeout)) {
			// Format: [ (Action ID << 4) + Response result(1：Success 0：Failure) ]
			uint8_t combined = response[1];
			result = combined & 0x0F; // Extract Success (1) / Failure (0) flag
			return true;
		}
		return false;
	}

	// 6. Request attitude of the remote device (used by camera)
	bool requestFcAttitude(RunCamAttitude& att, uint32_t timeout = 100) {
		uint8_t packet[3] = {
			RCDEVICE_PROTOCOL_HEADER,
			RCDEVICE_PROTOCOL_COMMAND_REQUEST_FC_ATTITUDE,
			0x00 // CRC placeholder
		};
		packet[2] = calculateCRC(packet, 2);
		_serial->write(packet, 3);

		uint8_t response[8];
		if (readPacket(response, 8, timeout)) {
			// Little-endian payload reconstruction
			att.roll  = ((uint16_t)response[2] << 8) | response[1];
			att.pitch = ((uint16_t)response[4] << 8) | response[3];
			att.yaw   = ((uint16_t)response[6] << 8) | response[5];
			return true;
		}
		return false;
	}

	// OSD: Fill Screen Area
	void fillRegion(uint8_t x, uint8_t y, uint8_t width, uint8_t height, char character) {
		uint8_t packet[8] = {
			RCDEVICE_PROTOCOL_HEADER,
			RCDEVICE_PROTOCOL_COMMAND_DISP_FILL_REGION,
			x, y, width, height,
			(uint8_t)character,
			 // CRC placeholder
		};
		packet[7] = calculateCRC(packet, 7);
		_serial->write(packet, 8);
	}

	// OSD: Write Single Character
	void writeChar(uint8_t x, uint8_t y, char character) {
		uint8_t packet[6] = {
			RCDEVICE_PROTOCOL_HEADER,
			RCDEVICE_PROTOCOL_COMMAND_DISP_WRITE_CHAR,
			x, y,
			(uint8_t)character,
			0x00 // CRC placeholder
		};
		packet[5] = calculateCRC(packet, 5);
		_serial->write(packet, 6);
	}

	// OSD: Write a String Horizontally
	void writeHorizontalString(uint8_t x, uint8_t y, const char* str) {
		uint8_t len = strlen(str);
		if (len > 60) len = 60; // Max length safe ceiling

		uint8_t packetSize = 6 + len;
		uint8_t* packet = new uint8_t[packetSize];

		packet[0] = RCDEVICE_PROTOCOL_HEADER;
		packet[1] = RCDEVICE_PROTOCOL_COMMAND_DISP_WRITE_HORT_STRING;
		packet[2] = len;
		packet[3] = x;
		packet[4] = y;
		memcpy(&packet[5], str, len);
		packet[packetSize - 1] = calculateCRC(packet, packetSize - 1);

		_serial->write(packet, packetSize);
		delete[] packet;
	}
};

#endif
