#pragma once

#include <Arduino.h>
#include <float16.h>


#define LORA_PACKET_LEN 200

//payload da trasmettere
// TODO: utlizza meglio lo spazio
typedef struct {
	uint8_t number; // order number
	uint8_t data[LORA_PACKET_LEN]; // FIXME
} LoRaPayload;


enum FrequencyBands {
	BAND_K = 0,
	BAND_L,
	BAND_M,
	BAND_N,
	BAND_O,
};

struct BandRequirements {
	float freq_start_mhz;
	int   ch_bw_khz;
	int   num_channels;
	int   max_power_mw;
	float max_duty;
	bool  polite_access;
};


void lora_setup(FrequencyBands band);
void lora_start_transmission(uint8_t* data, size_t size);
bool lora_is_transmission_done(void);

