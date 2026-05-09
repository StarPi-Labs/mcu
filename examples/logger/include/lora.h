#pragma once

#include <Arduino.h>
#include <float16.h>


enum LoRaPayloadType : uint8_t {
	PAYLOAD_1,
	PAYLOAD_2,
	PAYLOAD_3,
	PAYLOAD_STR
};

//payload da trasmettere
// TODO: utlizza meglio lo spazio
typedef union {
	struct {
		uint64_t timestamp;
		LoRaPayloadType type;
		union {
			struct {
				float16 roll, pitch, yaw;
				float16 vert_acc;
				float16 airbrake_angle;
			} p1;
			struct {
				float16 temp1, temp2, temp3;
				uint8_t para_state;
			} p2;
			struct {
				float16 pos_dx, pos_dy, pos_dz;
			} p3;
			char str[10];
		};
		uint8_t err_flag;
	} data;
	uint8_t bytes[sizeof(data)];
} LoRaPayload;



void lora_setup(void);
void lora_start_transmission(uint8_t* data, size_t size);
bool lora_is_transmission_done(void);

