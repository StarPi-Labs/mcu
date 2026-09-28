#pragma once

#include <Arduino.h>
#include <float16.h>

//payload da trasmettere
typedef union {
	struct { //dimensioni da capire (così si trasmette 40 pacchetti al secondo)
		float16 AccX;
		float16 AccY;
		float16 AccZ;
		float16 GyroX;
		float16 GyroY;
		float16 GyroZ;
		float16 EulerPitch;
		float16 EulerRoll;
		float16 EulerYaw;
		float16 AccX1;
		float16 AccY1;
		float16 AccZ1;
		float16 GyroX1;
		float16 GyroY1;
		float16 GyroZ1;
		float16 EulerPitch1;
		float16 EulerRoll1;
		float16 EulerYaw1;
	} values;
	uint8_t bytes[sizeof(values)];
} LoRaPayload;



void lora_setup(void);
void lora_start_transmission(uint8_t* data, size_t size);
bool lora_is_transmission_done(void);

