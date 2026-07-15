#pragma once

#include <Arduino.h>
#include <float16.h>


#define WAIT_TIMEOUT_MS    2
#define MAX_DRIFT_MS       5
#define CONNECT_TIMEOUT_MS 500
#define MAX_SYNC_MISSES    3
#define MAX_SILENT_FRAMES  5
#define LORA_FC_ID         0xFC
#define LORA_GS_ID         0xDE


enum LoRaPacketType : uint8_t
{
	PKT_UNKNOWN,
	PKT_SYNC,    // Broadcast sync packet GS -> FC
	PKT_CONNECT, // Connect request packet FC -> GS
	PKT_ACCEPT,  // Connect accept packet GS -> FC
	PKT_DATA,    // Data packet FC -> GS
	PKT_COMMAND, // Command packet GS -> FC
};

enum LoRaFCState : uint8_t
{
	STATE_DISCONNECTED,
	STATE_CONNECTING,
	STATE_TRANSMIT,
	STATE_RECEIVE,
};

enum LoRaTxMode {
	TX_NONE,
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


struct [[gnu::packed]] LoRaPacketHeader
{
	LoRaPacketType type;
	uint8_t id; // Sender ID
	uint8_t number; // Packet sequence number
	// TODO: specify endianness
	uint8_t tx_time[6]; // 48 bits timestamp in milliseconds since epoch
};

struct [[gnu::packed]] LoRaSyncPacket
{
	LoRaPacketHeader header;
	unsigned int connected : 1;       // 1 if FC is connected, 0 if not connected
	unsigned int sync_window : 10;    // ms between sync packets
	unsigned int gs_window : 9;       // ms reserved to the GS
	unsigned int security_window : 4; // ms reserved to the security window
	uint8_t padding[3];
};

struct [[gnu::packed]] LoRaConnectPacket
{
	LoRaPacketHeader header;
	uint8_t padding[6];
};

struct [[gnu::packed]] LoRaAcceptPacket
{
	LoRaPacketHeader header;
	uint8_t connect_time[6];
};

static_assert(sizeof(LoRaSyncPacket) == sizeof(LoRaConnectPacket), "LoRaSyncPacket and LoRaConnectPacket must be the same size");
static_assert(sizeof(LoRaSyncPacket) == sizeof(LoRaAcceptPacket), "LoRaSyncPacket and LoRaAcceptPacket must be the same size");

struct [[gnu::packed]] LoRaDataPacket
{
	LoRaPacketHeader header;
	struct {
		uint16_t altitude; // float16, changed to uint16_t for packing
		uint16_t vspeed;   // float16, changed to uint16_t for packing
		uint16_t attitude; // float16, changed to uint16_t for packing
		int16_t dt;
	} imu;
	struct {
		uint16_t p1;       // float16, changed to uint16_t for packing
		uint16_t p2;       // float16, changed to uint16_t for packing
		int16_t dt;
	} baro;
	struct {
		float latitude;
		float longitude;
		int16_t dt;
	} gps;
};

struct [[gnu::packed]] LoRaCommandPacket
{
	LoRaPacketHeader header;
	uint8_t command;
	uint64_t data;
};

struct BandRequirements {
	float freq_start_mhz;  // Start of the frequency band
	int   ch_bw_khz;       // (max) bandwidth that the channel supports
	int   num_channels;    // Number of channels given the bandwith
	int   max_power_mw;    // Maximum effective radiated power
	float max_duty;        // Maximum transmission duty cycle (see spec)
	bool  polite_access;   // Channel supports polite access for transmission
};


void lora_setup(FrequencyBands band, LoRaTxMode mode, uint8_t id, bool respect_power_limit = true);
bool lora_start_transmission(void *buffer, uint32_t len, int32_t time_window, LoRaTxMode tx_mode_override = TX_NONE);
bool lora_is_transmission_done(void);
void lora_enter_tx(void);
void lora_enter_rx(void);

void lora_prepare_next_packet(void);
LoRaDataPacket* lora_get_tx_packet(void);
void lora_release_tx_packet(void);

bool lora_is_channel_free(void);
void lora_start_receive(uint32_t timeout_ms);
bool lora_is_reception_done(void);

LoRaFCState lora_fc_state_machine(void);

uint64_t u48le_to_u64(uint8_t u48[6]);
void u64_to_u48le(uint64_t u64, uint8_t *u48);
