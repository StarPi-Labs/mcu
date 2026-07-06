#pragma once

#include <Arduino.h>
#include <float16.h>


const uint16_t FLIGHT_COMPUTER_ID = 0xf1ca;

// payload da trasmettere
typedef struct [[gnu::packed]] {
	uint16_t id; // ID of the device
	uint8_t number; // order number
	uint64_t tx_time;

	struct {
		uint16_t altitude; // float16, changed to uint16_t for packing
		uint16_t vspeed;   // float16, changed to uint16_t for packing
		uint16_t attitude; // float16, changed to uint16_t for packing
		int32_t dt;
	} imu;
	struct {
		uint16_t p1;       // float16, changed to uint16_t for packing
		uint16_t p2;       // float16, changed to uint16_t for packing
		int32_t dt;
	} baro;
	struct {
		float latitude;
		float longitude;
		int32_t dt;
	} gps;
} LoRaPayload;

// static_assert(sizeof(LoRaPayload) == 0, "");

enum LoRaTxMode {
	TX_FORCE,
	TX_DUTY,
	TX_POLITE,
};

enum FrequencyBands {
	BAND_K = 0,
	BAND_L,
	BAND_M,
	BAND_N,
	BAND_O,
};

struct BandRequirements {
	float freq_start_mhz;  // Start of the frequency band
	int   ch_bw_khz;       // (max) bandwidth that the channel supports
	int   num_channels;    // Number of channels given the bandwith
	int   max_power_mw;    // Maximum effective radiated power
	float max_duty;        // Maximum transmission duty cycle (see spec)
	bool  polite_access;   // Channel supports polite access for transmission
};


void lora_setup(FrequencyBands band, LoRaTxMode tx_mode);
void lora_start_transmission(void);
bool lora_is_transmission_done(void);
void lora_enter_tx(void);
void lora_enter_rx(void);

void lora_prepare_next_packet(uint8_t order_number);
LoRaPayload* lora_get_tx_packet(void);
void lora_release_tx_packet(void);
uint32_t lora_compute_dt(LoRaPayload *p, uint64_t time);

bool lora_is_channel_free(void);
void lora_start_receive(uint32_t timeout_ms);
bool lora_is_reception_done(void);
LoRaPayload lora_get_rx_packet(void);
