#pragma once

#include <Arduino.h>
#include <float16.h>


// payload da trasmettere
// TODO: da float a float16
typedef union {
	struct {
		struct {
			uint64_t timestamp;
			struct  {
				float altitude;
				float vspeed;
				float attitude;
				int32_t dt;
			} imu;
			struct {
				float alt1;
				float alt2;
				int32_t dt;
			} baro;
			struct {
				float latitude;
				float longitude;
				// TODO: time
			} gps;
		} sensor_data;
		uint8_t number; // order number
	}; // Data
	uint8_t padding[64]; // Force payload size to 64 bytes
} LoRaPayload;

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
void lora_update_imu_data(uint64_t time, float altitude, float vspeed, float attitude);
void lora_update_baro_data(uint64_t time, float alt1, float alt2);
void lora_update_gps_data(uint64_t time, float latitude, float longitude);

bool lora_is_channel_free(void);
void lora_start_receive(uint32_t timeout_ms);
bool lora_is_reception_done(void);
LoRaPayload lora_get_packet(void);
