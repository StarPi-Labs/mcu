/*
 * TODO:
 * 1. Make get_next_packet() independent of the packet type so that both the GS and FC
 *    can use the same mechanism
 * 2. Create RX and TX queues so that a packet can be queued and retransmitted
 * 3. Add a method for acknowledging packets, especially command packets
 *
 * FUTURE IMPROVEMENTS:
 * 1. The protocol can be simplified by employing slotted CSMA, since there are only
 *    two peers (the GS and FC) congestion should be manageable
 * 2. If master-slave syncronization is required then the protocol can be made
 *    master-polled, where the GS asks the FC for data waiting for a response
 * 3. This whole thing can be made into a class
 */

#include <Arduino.h>
#include <FreeRTOS.h>
#include <RadioLib.h>

#include "logger.h"
#include "lora.h"
#include "board.h"
#include "task.h"


extern SPIClass SPI2;
Module radio_module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY, SPI2); //nello stack anziché heap
SX1262 radio(&radio_module);

static volatile bool tx_operation_done = false;
static volatile bool rx_operation_done = false;
static volatile uint64_t last_tx_time = 0;
static volatile uint64_t last_rx_time = 0;
static int last_tx_toa = 0;

#define LORA_MAX_PAYLOAD 255
static uint8_t rx_buffer[LORA_MAX_PAYLOAD];
static uint32_t rx_len = 0;

// RX commands will be pushed to this queue and the handler task will be signaled
static QueueHandle_t rx_cmd_queue;
static TaskHandle_t  rx_cmd_task_handle;

static LoRaDataPacket next_packet;
static LoRaTxMode tx_mode;
static uint8_t next_order_number = 0;
static uint8_t machine_id = 0;

static FrequencyBands freq_band;
static bool is_tx = true;

DECLARE_STATIC_SEMAPHORE(next_packet_mutex);


// As per EN 300 220-2 V3.3.1; Annex B table 1
// https://www.etsi.org/deliver/etsi_en/300200_300299/30022002/03.03.01_60/en_30022002v030301p.pdf
const struct BandRequirements EU_868_BANDS[] = {
	[BAND_K] = {.freq_start_mhz = 863.0, .ch_bw_khz = 500, .num_channels = 4, .max_power_mw = 25,  .max_duty = 0.1, .polite_access = true},
	[BAND_L] = {.freq_start_mhz = 865.0, .ch_bw_khz = 500, .num_channels = 6, .max_power_mw = 25,  .max_duty = 1,   .polite_access = true},
	[BAND_M] = {.freq_start_mhz = 868.0, .ch_bw_khz = 500, .num_channels = 1, .max_power_mw = 25,  .max_duty = 1,   .polite_access = true},
	[BAND_N] = {.freq_start_mhz = 868.7, .ch_bw_khz = 500, .num_channels = 1, .max_power_mw = 25,  .max_duty = 0.1, .polite_access = true},
	[BAND_O] = {.freq_start_mhz = 869.4, .ch_bw_khz = 250, .num_channels = 1, .max_power_mw = 500, .max_duty = 10,  .polite_access = true},
};


// Ebyte E22-900M33S module power output, since it has an LNA and a PA,
// the actual output power is not the same as the value passed to RadioLib's
// setOutputPower() function. The table below maps the two values.
// Note that this is the output power of the module, not the effective radiated
// power (ERP) of the antenna.
// Refer to image in page 10, chapter 4.2 of the manual: https://www.cdebyte.com/products/E22-900M33S/4#Downloads
// Useful to extract data: https://plotdigitizer.com/app
// NOTE: THE X SCALE ON THIS GRAPH IS NOT CONSTANT, WHY????
const struct {
	uint16_t power_mw;     // The actual power in mW, for reference
	int8_t radiolib_value; // The value to pass to RadioLib's setOutputPower() function
} LORA_OUTPUT_POWER_TABLE[] = {
	{ .power_mw = 59,   .radiolib_value = -9 },
	{ .power_mw = 75,   .radiolib_value = -8 },
	{ .power_mw = 98,   .radiolib_value = -7 },
	{ .power_mw = 126,  .radiolib_value = -6 },
	{ .power_mw = 173,  .radiolib_value = -5 },
	{ .power_mw = 237,  .radiolib_value = -4 },
	{ .power_mw = 299,  .radiolib_value = -3 },
	{ .power_mw = 390,  .radiolib_value = -2 },
	{ .power_mw = 487,  .radiolib_value = -1 },
	{ .power_mw = 619,  .radiolib_value =  0 },
	{ .power_mw = 750,  .radiolib_value =  1 },
	{ .power_mw = 1044, .radiolib_value =  2 },
	{ .power_mw = 1202, .radiolib_value =  3 },
	{ .power_mw = 1409, .radiolib_value =  4 },
	{ .power_mw = 1492, .radiolib_value =  5 },
	{ .power_mw = 1585, .radiolib_value =  6 },
	{ .power_mw = 1745, .radiolib_value =  7 },
	{ .power_mw = 2218, .radiolib_value =  8 },
};


// TODO: all timing in lora packets could happen in local time since boot, right
// now instead all timings are milliseconds since epoch. NOTE that the time is
// only accurate once the GPS module has received a valid fix.

static void tx_operation_done_cb(void)
{
	last_tx_time = now_ms();
	tx_operation_done = true;
}

static void rx_operation_done_cb(void)
{
	last_rx_time = now_ms();
	rx_operation_done = true;
}


void lora_setup(FrequencyBands band, LoRaTxMode mode, uint8_t id, bool respect_power_limit)
{
	int state = radio.begin();
	if (state != RADIOLIB_ERR_NONE) {
		// FIXME: log state
		log(S_LORA, T_SYSLOG, "[ERR] failed to initialize radio");
		while (true);
	}

	freq_band = band;
	machine_id = id;

	if (mode == TX_DUTY && EU_868_BANDS[band].polite_access) {
		tx_mode = TX_POLITE;
		log(S_LORA, T_SYSLOG, "Upgrading from TX_DUTY to TX_POLITE");
	} else if (mode == TX_POLITE && !EU_868_BANDS[band].polite_access) {
		tx_mode = TX_DUTY;
		log(S_LORA, T_SYSLOG, "polite access not supported in this band, using TX_DUTY instead");
	} else {
		tx_mode = mode;
	}

	radio.setDio2AsRfSwitch(true);

	radio.setFrequency(EU_868_BANDS[band].freq_start_mhz + (float)EU_868_BANDS[band].ch_bw_khz/2000.0); // TODO: switch channel
	radio.setBandwidth(EU_868_BANDS[band].ch_bw_khz);

	if (respect_power_limit) {
		// find the closest power value in the table that is less than or equal to the max power
		int8_t radiolib_power = LORA_OUTPUT_POWER_TABLE[0].radiolib_value;
		for (size_t i = 0; i < sizeof(LORA_OUTPUT_POWER_TABLE)/sizeof(LORA_OUTPUT_POWER_TABLE[0]); i++) {
			if (LORA_OUTPUT_POWER_TABLE[i].power_mw <= EU_868_BANDS[band].max_power_mw) {
				radiolib_power = LORA_OUTPUT_POWER_TABLE[i].radiolib_value;
			} else {
				break;
			}
		}
		radio.setOutputPower(radiolib_power);
	} else {
		radio.setOutputPower(LORA_OUTPUT_POWER);
	}

	radio.setSpreadingFactor(LORA_SPREADING_FACTOR);
	radio.setCodingRate(LORA_CODING_RATE);
	radio.forceLDRO(false);
	radio.setCRC(LORA_CRC_BYTES);

	/* DIO1 cannot be set as both the transmission done and receprion done
	 * interrupt so we need to implement a mode switch behavior which reassigns
	 * the interrupt pin with a different callback */
	is_tx = true;
	radio.setPacketSentAction(tx_operation_done_cb);

	INIT_STATIC_SEMAPHORE(next_packet_mutex);
	if (next_packet_mutex == NULL) {
		log(S_LORA, T_SYSLOG, "[ERR] failed to create next_packet mutex");
		while (true);
	}

	// Abilita la trasmissione e ricezione del primo pacchetto
	tx_operation_done = true;
	rx_operation_done = true;
	next_order_number = 0;
	lora_prepare_next_packet();
}


uint64_t u48le_to_u64(uint8_t u48[6])
{
	return ((uint64_t)u48[0] << 0) |
	       ((uint64_t)u48[1] << 8) |
	       ((uint64_t)u48[2] << 16) |
	       ((uint64_t)u48[3] << 24) |
	       ((uint64_t)u48[4] << 32) |
	       ((uint64_t)u48[5] << 40);
}

void u64_to_u48le(uint64_t u64, uint8_t *u48)
{
	u48[0] = (uint8_t)(u64 >> 0);
	u48[1] = (uint8_t)(u64 >> 8);
	u48[2] = (uint8_t)(u64 >> 16);
	u48[3] = (uint8_t)(u64 >> 24);
	u48[4] = (uint8_t)(u64 >> 32);
	u48[5] = (uint8_t)(u64 >> 40);
}

// FIXME
static void adjust_time(void *p)
{
	switch (((LoRaPacketHeader*)p)->type) {
	case PKT_DATA: {
		LoRaDataPacket *d = (LoRaDataPacket*)p;
		// adjust time from packet creation to packet transmission start
		uint64_t base = u48le_to_u64(d->header.tx_time);
		uint64_t now = now_ms();
		u64_to_u48le(now, d->header.tx_time);
		d->imu.dt = d->imu.dt + base - now;
		d->baro.dt = d->baro.dt + base - now;
		d->gps.dt = d->gps.dt + base - now; // FIXME: GPS dt should be in decisenconds or smt
		break;
	}
	default:
		u64_to_u48le(now_ms(), ((LoRaPacketHeader*)p)->tx_time);
		break;
	}
}


void lora_prepare_next_packet(void)
{
	xSemaphoreTake(next_packet_mutex, portMAX_DELAY);
	memset(&next_packet, 0, sizeof(next_packet));
	next_packet.header.type = PKT_DATA;
	u64_to_u48le(now_ms(), next_packet.header.tx_time);
	xSemaphoreGive(next_packet_mutex);
}


LoRaDataPacket* lora_get_tx_packet(void)
{
	xSemaphoreTake(next_packet_mutex, portMAX_DELAY);
	return &next_packet;
}


void lora_release_tx_packet(void)
{
	xSemaphoreGive(next_packet_mutex);
}


bool lora_is_channel_free(void)
{
	int16_t state = radio.scanChannel();
	return state == RADIOLIB_CHANNEL_FREE;
}


// Wait for a packet to be received or timeout, return true if packet received, false otherwise
bool lora_receive_timeout(int64_t timeout_ms)
{
	int64_t start_time = millis();

	// If not in transmit mode, wait for the operation done and enter
	if (is_tx == true) {
		// Ignore the result since we just want to switch to RX mode
		(void)radio.finishTransmit();
		is_tx = false;
		radio.setPacketReceivedAction(rx_operation_done_cb);
	}

	// start receive operation
	rx_operation_done = false;
	if (radio.startReceive() != RADIOLIB_ERR_NONE) {
		rx_operation_done = true;
		return false;
	}


	// Wait for the packet to be received or timeout
	while (rx_operation_done == false) {
		if (millis() - start_time > timeout_ms) {
			break;
		}
		vTaskDelay(pdMS_TO_TICKS(WAIT_TIMEOUT_MS));
	}
	if (radio.finishReceive() != RADIOLIB_ERR_NONE) {
		rx_operation_done = true;
	}

	// Timeout occurred
	if (rx_operation_done == false) {
		return false;
	}

	// Parse the packet and check for integrity
	rx_len = radio.getPacketLength();
	if (rx_len > LORA_MAX_PAYLOAD) {
		rx_len = LORA_MAX_PAYLOAD;
	}
	if (rx_len == 0 || radio.readData(rx_buffer, rx_len) != RADIOLIB_ERR_NONE) {
		rx_len = 0;
		return false;
	}

	return true;
}


// Transmit a packet with a timeout, return true if successful, false otherwise
bool lora_transmit_timeout(void *buffer, uint32_t len, int64_t timeout_ms, LoRaTxMode tx_mode_override)
{
	int64_t start_time = millis();
	int64_t toa_ms = radio.getTimeOnAir(len)/1000;

	// Enter TX mode if not already in TX mode
	if (is_tx == false) {
		// Call to cleanup any pending reception
		// Ignore any errors, as we are switching modes anyway
		(void)radio.finishReceive();
		// We don't know if a packet is going to arrive, so switch mode immediately
		is_tx = true;
		radio.setPacketSentAction(tx_operation_done_cb);
	}

	// Wait according to the tx mode
	if (tx_mode_override == TX_NONE) tx_mode_override = tx_mode;

	switch (tx_mode_override) {
	case TX_FORCE:
		break;
	// ETSI EN 300 220-2 V3.3.1 Annex B defines the band duty cycles
	// (max_duty as a percentage) and requires polite (LBT) access where
	// polite_access == true. The CCA backoff range 5-50ms and max 10
	// retries follow common LBT practice for SRD in the 863-870 MHz band.
	case TX_POLITE: {
		const int max_cca = 10;
		randomSeed(start_time);
		for (int i = 0; i < max_cca; i++) {
			if (lora_is_channel_free()) {
				break;
			}
			vTaskDelay(pdMS_TO_TICKS(random(5, 50)));
		}
		// if the channel is still busy after max_cca attempts, we will transmit anyway
		break;
	}
	// Duty cycle enforcement: the band's max_duty is a percentage, so we
	// divide by 100 to get the fraction. The minimum interval between
	// transmission starts is max_toa_us / duty, derived from the definition
	// duty = Tx_time / interval. For a fixed packet size this guarantees
	// the long-term average stays within the regulatory limit.
	case TX_DUTY: {
		float duty = EU_868_BANDS[freq_band].max_duty / 100.0f;
		if (duty <= 0.0f) duty = 1.0f;
		uint64_t min_interval_us = (uint64_t)((float)last_tx_toa / duty); // FIXME
		uint64_t now = now_us();
		uint64_t next_allowed = last_tx_time*1000 + min_interval_us;

		if (now < next_allowed) {
			vTaskDelay(pdMS_TO_TICKS((next_allowed - now) / 1000) + 1);
		}
		break;
	}
	default:
		log(S_LORA, T_SYSLOG, "[ERR]: invalid tx mode");
		return false;
		break;
	}

	// Done waiting, check if the time window has been exceeded
	if (millis() - start_time > timeout_ms - toa_ms) {
		// ABORT: time window exceeded
		return false;
	}

	// Transmit the packet
	((LoRaPacketHeader*)buffer)->number = next_order_number++;
	((LoRaPacketHeader*)buffer)->id = machine_id;
	adjust_time(buffer);
	last_tx_toa = toa_ms;
	tx_operation_done = false;
	if (radio.startTransmit((uint8_t*)buffer, len) != RADIOLIB_ERR_NONE) {
		return false;
	}

	// Wait for the transmission to complete, max timeout is 2*expected time on air
	start_time = millis();
	while (tx_operation_done == false) {
		vTaskDelay(pdMS_TO_TICKS(WAIT_TIMEOUT_MS));
		if (millis() - start_time > toa_ms*2) {
			// Transmission took too long, maybe IRQ was lost
			return false;
		}
	}
	if (radio.finishTransmit() != RADIOLIB_ERR_NONE) {
		return false;
	}

	return true;
}


void lora_set_rx_cmd_task_handle(TaskHandle_t handle)
{
	rx_cmd_task_handle = handle;
}


void lora_set_rx_cmd_queue(QueueHandle_t handle)
{
	rx_cmd_queue = handle;
}


/*
┌───────────────┐              ┌───────────────┐
│GROUND STATION │              │FLIGHT COMPUTER│
└───────┬───────┘              └───────┬───────┘
        │          Handshake           │
     ┌ ─│─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┼ ─ ┐
        ├────────────SYNC─────────────>│
     │  │                              │   │
        │<──────────CONNECT────────────┤
     │  │                              │   │
        ├───────────ACCEPT────────────>│
     └ ─│─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┼ ─ ┘
        ├───────────COMMAND───────────>│
        │                              │
        │░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│Security Window
        │                              │
        │<──────────TELEMETRY──────────┤
        │<──────────TELEMETRY──────────┤
        │                              │
        │              .               │
        │              .               │
        │              .               │
        │                              │
        │<──────────TELEMETRY──────────┤
        │<──────────TELEMETRY──────────┤
        │<──────────TELEMETRY──────────┤
        │                              │
        │░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│Security Window
     G  ├─────────────SYNC────────────>├ ─ ─ ─
     S  ├────────────COMMAND──────────>│    ^
        │              .               │    │
     W  │              .               │    │
     I  │              .               │    │
     N  ├────────────COMMAND──────────>│    │
     D  ├────────────COMMAND──────────>│    S
     O  │                              │    Y
     W  │░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│    N
        │                              │    C
        │<──────────TELEMETRY──────────┤
        │<──────────TELEMETRY──────────┤    W
        │                              │    I
        │              .               │    N
        │              .               │    D
        │              .               │    O
        │                              │    W
        │<──────────TELEMETRY──────────┤    │
        │<──────────TELEMETRY──────────┤    │
        │<──────────TELEMETRY──────────┤    │
        │                              │    │
        │░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│    v
        ├─────────────SYNC────────────>├ ─ ─ ─
        │                              │
        │                              │
 */

static LoRaPacketHeader lora_get_header()
{
	LoRaPacketHeader header;
	memcpy(&header, rx_buffer, sizeof(LoRaPacketHeader));
	return header;
}

static LoRaSyncPacket lora_get_sync()
{
	LoRaSyncPacket sync;
	memcpy(&sync, rx_buffer, sizeof(LoRaSyncPacket));
	return sync;
}

static LoRaAcceptPacket lora_get_accept()
{
	LoRaAcceptPacket accept;
	memcpy(&accept, rx_buffer, sizeof(LoRaAcceptPacket));
	return accept;
}


static LoRaCommandPacket lora_get_command()
{
	LoRaCommandPacket cmd;
	memcpy(&cmd, rx_buffer, sizeof(LoRaCommandPacket));
	return cmd;
}


static int64_t slot_relative_time(int64_t last_sync_time, int64_t delta = 0)
{
	return now_ms() + delta - last_sync_time;
}


LoRaFCState lora_fc_state_machine()
{
	static LoRaFCState state = STATE_DISCONNECTED;
	static int64_t  clock_delta     = 0; // clock delta between FC and GS
	static uint64_t sync_rx_time    = 0; // absolute time of the last sync packet received in ms
	static uint64_t sync_tx_time    = 0; // absolute time of the last sync packet transmitted in ms
	static uint64_t sync_time       = 0; // absolute time of the last sync packet received in ms
	static uint64_t sync_trip_time  = 0; // time it takes for a single sync packet takes to arrive in ms
	static uint32_t sync_window     = 0; // time window between sync packets in ms
	static uint32_t gs_window       = 0; // time window for GS packets in ms
	static uint32_t security_window = 0; // silent time window in ms
	static uint32_t sync_misses     = 0; // number of sync packet misses
	static bool     sync_received   = false; // true if we have received a sync packet

	/*
	 * Connection Handshake:
	 * 1. The GS broadcasts a SYNC packet containing the absolute time it was sent
	 * 2. The FC receives the SYNC packet and responds with a CONNECT packet
	 * 3. The GS receives the CONNECT packet and responds with a ACCEPT packet
	 *    containing the absolute time the CONNECT packet was received
	 * 4. The FC calculates the clock delta and both are connected
	 */


	switch (state) {
	case STATE_DISCONNECTED: {
		clock_delta     = 0;
		sync_rx_time    = 0;
		sync_tx_time    = 0;
		sync_time       = 0;
		sync_trip_time  = 0;
		sync_window     = 0;
		gs_window       = 0;
		security_window = 0;
		sync_misses     = 0;
		sync_received   = false;

		if (lora_receive_timeout(2000) == false) break;

		LoRaPacketHeader header = lora_get_header();
		if (header.type == PKT_SYNC && header.id == LORA_GS_ID) {
			sync_rx_time = last_rx_time;
			sync_tx_time = u48le_to_u64(header.tx_time);

			LoRaSyncPacket sync = lora_get_sync();
			log(S_LORA, T_SYSLOG, "sync packet received");

			sync_window = sync.sync_window;
			gs_window = sync.gs_window;
			security_window = sync.security_window;

			// Received first sync packet from GS, start the handshake
			state = STATE_CONNECTING;
		}
		break;
	}
	case STATE_CONNECTING: {
		// Send a CONNECT packet to the GS
		LoRaConnectPacket p = {};
		p.header.type = PKT_CONNECT;

		uint32_t timeout = sync_window/2;
		uint64_t connect_tx_time = 0;
		uint64_t connect_rx_time = 0;

		if (lora_transmit_timeout(&p, sizeof(p), timeout, TX_FORCE) == false) {
			state = STATE_DISCONNECTED;
			break;
		}
		connect_tx_time = last_tx_time;

		// Wait for the GS to respond with an accept packet
		if (lora_receive_timeout(timeout) == false) {
			state = STATE_DISCONNECTED;
			break;
		}

		LoRaPacketHeader header = lora_get_header();
		if (header.type == PKT_ACCEPT && header.id == LORA_GS_ID) {
			connect_rx_time = u48le_to_u64(lora_get_accept().connect_time);

			// Delta computation
			// https://en.wikipedia.org/wiki/Cristian%27s_algorithm
			// https://www.analog.com/en/resources/analog-dialogue/articles/clock-synchro-with-ieee-1588-and-blackfin.html
			clock_delta = -((sync_rx_time - sync_tx_time) - (connect_tx_time - connect_rx_time)) / 2;
			// FIXME: the single trip time a sync packet takes could be transmitted by the master and
			// the error would be less (just the propagation delay), this instead takes in account
			// the time it takes to transmit the sync packet and the time it takes to receive it back
			// plus the two propagation delays
			sync_trip_time = ((connect_rx_time - sync_tx_time) - (connect_tx_time - sync_rx_time)) / 2;
			sync_time = sync_tx_time;

			state = STATE_RECEIVE; // first slot is reserved to FC transmission
		} else {
			state = STATE_DISCONNECTED;
		}
		break;
	}

	case STATE_TRANSMIT: {
		uint32_t toa = radio.getTimeOnAir(sizeof(LoRaDataPacket))/1000;
		int64_t slot = slot_relative_time(sync_time, clock_delta);
		int64_t remaining_time = sync_window - security_window - slot;

		// In the receive window, switch to receive mode
		if (slot <= gs_window) {
			sync_received = false;
			state = STATE_RECEIVE;
			break;
		}

		// If the packet would arrive after the tx window of the fc switch to receive mode
		if (remaining_time - toa < 0) {
			sync_received = false;
			state = STATE_RECEIVE;
			break;
		}

		LoRaDataPacket tx_packet;
		xSemaphoreTake(next_packet_mutex, portMAX_DELAY);
		memcpy(&tx_packet, &next_packet, sizeof(tx_packet));
		xSemaphoreGive(next_packet_mutex);

		lora_transmit_timeout(&tx_packet, sizeof(tx_packet), remaining_time - toa, TX_NONE);
		// TODO: check and log errors

		break;
	}

	case STATE_RECEIVE: {
		// Receive for the ground station window, including the security window to avoid
		// switching too early or loosing packets
		int64_t remaining_time = gs_window - slot_relative_time(sync_time, clock_delta);

		if (remaining_time < 0) {
			if (sync_received == false) {
				sync_misses++;
			}
			if (sync_misses > MAX_SYNC_MISSES) {
				state = STATE_DISCONNECTED;
			} else {
				state = STATE_TRANSMIT;
			}
			break;
		}

		if (lora_receive_timeout(remaining_time) == false) {
			if (sync_received == false) {
				sync_misses++;
			}
			if (sync_misses > MAX_SYNC_MISSES) {
				state = STATE_DISCONNECTED;
			} else {
				state = STATE_TRANSMIT;
			}
			break;
		}

		LoRaPacketHeader p = lora_get_header();
		if (p.id != LORA_GS_ID) break;

		switch (p.type) {
		case PKT_SYNC: {
			sync_received = true;
			sync_misses = 0;
			sync_rx_time = last_rx_time;
			sync_tx_time = u48le_to_u64(p.tx_time);
			sync_time = sync_tx_time;
			LoRaSyncPacket s = lora_get_sync();
			if (s.connected == false) {
				// ground station thinks we are not connected
				state = STATE_DISCONNECTED;
				break;
			}
			int64_t drift = sync_rx_time - clock_delta - sync_tx_time - sync_trip_time;
			if (llabs(drift) > MAX_DRIFT_MS) {
				clock_delta += drift;
			}

			// update window parameters from sync packet
			sync_window = s.sync_window;
			gs_window = s.gs_window;
			security_window = s.security_window;
			break;
		}
		case PKT_COMMAND: {
			// TODO: handle command packet
			LoRaCommandPacket c = lora_get_command();
			xQueueSendToBack(rx_cmd_queue, &(c.data), 0);
			xTaskNotifyGive(rx_cmd_task_handle);
			break;
		}
		default:
			// FIXME: should we disconnect?
			break;
		}

		break;
	}

	default:
		state = STATE_DISCONNECTED;
		break;
	}

	return state;
}


LoRaFCState lora_gs_state_machine()
{
	static LoRaFCState state = STATE_DISCONNECTED;
	static int64_t  sync_sent_time  = 0; // absolute time of the last sync packet sent in ms
	static int64_t  connect_rx_time = 0; // absolute time of the last connect packet received in ms
	static int      packets_received = 0; // number of packets received from FC
	static int      silent_frames    = 0;

	static const int32_t sync_window     = 1000; // time window between sync packets in ms
	static const int32_t gs_window       = 200;  // time window for GS packets in ms
	static const int32_t security_window = 10;   // silent time window in ms

	switch (state) {
	case STATE_DISCONNECTED: {
		connect_rx_time = 0;
		packets_received = 0;
		silent_frames = 0;

		if ((now_ms() - sync_sent_time) >= sync_window) {
			// Send sync packet to FC
			LoRaSyncPacket s = {};
			s.header.type = PKT_SYNC;
			s.sync_window = sync_window;
			s.gs_window = gs_window;
			s.security_window = security_window;
			s.connected = false;

			if (lora_transmit_timeout(&s, sizeof(s), sync_window, TX_FORCE)) {
				sync_sent_time = last_tx_time;
			}
			break;
		} else {
			if (lora_receive_timeout(sync_window - (slot_relative_time(sync_sent_time))) == false) {
				// No connect received within the sync window, return to disconnected state
				break;
			}

			LoRaPacketHeader header = lora_get_header();
			if (header.type == PKT_CONNECT && header.id == LORA_FC_ID) {
				// Received first sync packet from GS, start the handshake
				connect_rx_time = last_rx_time;
				state = STATE_CONNECTING;
				break;
			}
		}
		break;
	}

	case STATE_CONNECTING: {
		// Send a ACCEPT packet to the GS
		LoRaAcceptPacket a = {};
		a.header.type = PKT_ACCEPT;
		u64_to_u48le(connect_rx_time, a.connect_time);

		if (lora_transmit_timeout(&a, sizeof(a), sync_window/2, TX_FORCE) == false) {
			state = STATE_DISCONNECTED;
			break;
		}

		state = STATE_TRANSMIT;
		break;
	}

	case STATE_TRANSMIT: {

		if (silent_frames >= MAX_SILENT_FRAMES) {
			state = STATE_DISCONNECTED;
			break;
		}

		// Sync window expired, need to resend sync packet, this should only happen
		// once when entering STATE_TRANSMIT after the first window after the handshake
		if (slot_relative_time(sync_sent_time) >= sync_window) {
			LoRaSyncPacket s = {};
			s.header.type = PKT_SYNC;
			s.sync_window = sync_window;
			s.gs_window = gs_window;
			s.security_window = security_window;
			s.connected = true;

			if (lora_transmit_timeout(&s, sizeof(s), sync_window, TX_FORCE) == false) {
				state = STATE_DISCONNECTED;
				break;
			}
			sync_sent_time = last_tx_time;
			break;
		}

		int64_t remaining_time = gs_window - security_window - slot_relative_time(sync_sent_time);

		if (remaining_time <= 0) {
			packets_received = 0;
			state = STATE_RECEIVE;
			break;
		}

		// Switched to early to transmit, wait for remaining listen time to expire
		if (remaining_time >= gs_window) {
			vTaskDelay(pdMS_TO_TICKS(remaining_time-gs_window));
			break;
		}

		// TODO: transmit commands to FC

		break;
	}

	case STATE_RECEIVE: {
		int64_t remaining_time = sync_window - security_window - slot_relative_time(sync_sent_time);

		// In the transmit window
		if (remaining_time <= 0) {
			if (packets_received == 0) {
				silent_frames++;
			}

			state = STATE_TRANSMIT;
			break;
		}

		// In the current frame's transmit window, we shouldn't be here yet
		if (remaining_time >= sync_window - gs_window + security_window) {
			if (packets_received == 0) {
				silent_frames++;
			}

			state = STATE_TRANSMIT;
			break;
		}

		if (lora_receive_timeout(remaining_time) == false) {
			if (packets_received == 0) {
				silent_frames++;
			}
			state = STATE_TRANSMIT;
			break;
		} else {
			packets_received++;
			silent_frames = 0;
			// TODO: do something with the packet
		}

		break;
	}

	default:
		state = STATE_DISCONNECTED;
		break;
	}

	return state;
}
