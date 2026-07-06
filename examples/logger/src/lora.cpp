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

volatile bool tx_operation_done = false;
volatile bool rx_operation_done = false;

static uint8_t rx_buffer[sizeof(LoRaPayload)];
static LoRaPayload next_packet;
static LoRaTxMode tx_mode;
static FrequencyBands freq_band;
static bool is_tx = true;

static uint32_t max_toa_us = 0;
static uint64_t last_tx_start_us = 0;

DECLARE_STATIC_SEMAPHORE(next_packet_mutex);


// As per EN 300 220-2 V3.3.1; Annex B table 1
// https://www.etsi.org/deliver/etsi_en/300200_300299/30022002/03.03.01_60/en_30022002v030301p.pdf
const struct BandRequirements bands[] = {
	[BAND_K] = {.freq_start_mhz = 863.0, .ch_bw_khz = 500, .num_channels = 4, .max_power_mw = 25,  .max_duty = 0.1, .polite_access = true},
	[BAND_L] = {.freq_start_mhz = 865.0, .ch_bw_khz = 500, .num_channels = 6, .max_power_mw = 25,  .max_duty = 1,   .polite_access = true},
	[BAND_M] = {.freq_start_mhz = 868.0, .ch_bw_khz = 500, .num_channels = 1, .max_power_mw = 25,  .max_duty = 1,   .polite_access = true},
	[BAND_N] = {.freq_start_mhz = 868.7, .ch_bw_khz = 500, .num_channels = 1, .max_power_mw = 25,  .max_duty = 0.1, .polite_access = true},
	[BAND_O] = {.freq_start_mhz = 869.4, .ch_bw_khz = 250, .num_channels = 1, .max_power_mw = 500, .max_duty = 10,  .polite_access = true},
};


static void tx_operation_done_cb(void)
{
	tx_operation_done = true;
}

static void rx_operation_done_cb(void)
{
	rx_operation_done = true;
}


void lora_setup(FrequencyBands band, LoRaTxMode mode)
{
	int state = radio.begin();
	if (state != RADIOLIB_ERR_NONE) {
		// FIXME: log state
		log(S_LORA, T_SYSLOG, "[ERR] failed to initialize radio");
		while (true);
	}

	freq_band = band;

	if (mode == TX_DUTY && bands[band].polite_access) {
		tx_mode = TX_POLITE;
		log(S_LORA, T_SYSLOG, "Upgrading from TX_DUTY to TX_POLITE");
	} else if (mode == TX_POLITE && !bands[band].polite_access) {
		tx_mode = TX_POLITE;
		log(S_LORA, T_SYSLOG, "polite access not supported in this band, using TX_DUTY instead");
	} else {
		tx_mode = mode;
	}

	radio.setDio2AsRfSwitch(true);
	radio.setFrequency(bands[band].freq_start_mhz);
	radio.setOutputPower(LORA_OUTPUT_POWER); // TODO: limit power
	radio.setBandwidth(bands[band].ch_bw_khz); // TODO: switch channel
	radio.setSpreadingFactor(LORA_SPREADING_FACTOR);
	radio.setCodingRate(LORA_CODING_RATE);
	radio.forceLDRO(false);
	radio.implicitHeader(sizeof(LoRaPayload)); // PACCHETTI A LUNGHEZZA FISSA
	radio.setCRC(LORA_CRC_BYTES);

	/* DIO1 cannot be set as both the transmission done and receprion done
	 * interrupt so we need to implement a mode switch behavior which reassigns
	 * the interrupt pin with a different callback */
	is_tx = true;
	radio.setPacketSentAction(tx_operation_done_cb);

	max_toa_us = radio.getTimeOnAir(sizeof(LoRaPayload));
	last_tx_start_us = now_us();

	INIT_STATIC_SEMAPHORE(next_packet_mutex);
	if (next_packet_mutex == NULL) {
		log(S_LORA, T_SYSLOG, "[ERR] failed to create next_packet mutex");
		while (true);
	}

	//primo pacchetto
	//start_transmission(LoRaPayload{0}.bytes, sizeof(LoRaPayload));

	// Abilita la trasmissione e ricezione del primo pacchetto
	tx_operation_done = true;
	rx_operation_done = true;
	lora_prepare_next_packet(0);
}


void lora_enter_tx(void)
{
	// If not in transmit mode, wait for the operation done and enter
	if (is_tx == false) {
		while(lora_is_reception_done() == false) {
			vTaskDelay(10); // TODO: change wait amount
		}
		is_tx = true;
		radio.setPacketSentAction(tx_operation_done_cb);
	}
}


void lora_enter_rx(void)
{
	// If not in transmit mode, wait for the operation done and enter
	if (is_tx == true) {
		while(lora_is_transmission_done() == false) {
			vTaskDelay(10); // TODO: change wait amount
		}
		is_tx = false;
		radio.setPacketReceivedAction(rx_operation_done_cb);
	}
}


static void adjust_dt(LoRaPayload *p)
{
	// adjust time from packet creation to packet transmission start
	uint64_t base = p->tx_time;
	p->tx_time = now_us();
	p->imu.dt = lora_compute_dt(p, p->imu.dt + base);
	p->baro.dt = lora_compute_dt(p, p->baro.dt + base);
}


void lora_start_transmission()
{
	static LoRaPayload tx_packet;

	xSemaphoreTake(next_packet_mutex, portMAX_DELAY);
	memcpy(&tx_packet, &next_packet, sizeof(tx_packet));
	xSemaphoreGive(next_packet_mutex);

	switch (tx_mode) {
	case TX_FORCE:
		adjust_dt(&tx_packet);
		tx_operation_done = false;
		radio.startTransmit((uint8_t*)&tx_packet, sizeof(tx_packet));
		break;
	// ETSI EN 300 220-2 V3.3.1 Annex B defines the band duty cycles
	// (max_duty as a percentage) and requires polite (LBT) access where
	// polite_access == true. The CCA backoff range 5-50ms and max 10
	// retries follow common LBT practice for SRD in the 863-870 MHz band.
	case TX_POLITE: {
		const int max_cca = 10;
		for (int i = 0; i < max_cca; i++) {
			if (lora_is_channel_free()) {
				break;
			}
			vTaskDelay(pdMS_TO_TICKS(random(5, 51)));
		}
		// if the channel is still busy after max_cca attempts, we will transmit anyway
		adjust_dt(&tx_packet);
		tx_operation_done = false;
		radio.startTransmit((uint8_t*)&tx_packet, sizeof(tx_packet));
		break;
	}
	// Duty cycle enforcement: the band's max_duty is a percentage, so we
	// divide by 100 to get the fraction. The minimum interval between
	// transmission starts is max_toa_us / duty, derived from the definition
	// duty = Tx_time / interval. For a fixed packet size this guarantees
	// the long-term average stays within the regulatory limit.
	case TX_DUTY: {
		float duty = bands[freq_band].max_duty / 100.0f;
		if (duty <= 0.0f) duty = 1.0f;
		uint64_t min_interval_us = (uint64_t)((float)max_toa_us / duty);
		uint64_t now = now_us();
		uint64_t next_allowed = last_tx_start_us + min_interval_us;

		if (now < next_allowed) {
			vTaskDelay(pdMS_TO_TICKS((next_allowed - now) / 1000) + 1);
		}

		adjust_dt(&tx_packet);
		tx_operation_done = false;
		radio.startTransmit((uint8_t*)&tx_packet, sizeof(tx_packet));
		last_tx_start_us = now_us();
		break;
	}
	default:
		log(S_LORA, T_SYSLOG, "[ERR]: invalid tx mode");
		break;
	}
}


bool lora_is_transmission_done(void)
{
	if (is_tx == false) {
		return false;
	}

	if (tx_operation_done) {
		radio.finishTransmit();
	}
	return tx_operation_done;
}


void lora_prepare_next_packet(uint8_t order_number)
{
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	xSemaphoreTake(next_packet_mutex, portMAX_DELAY);
	memset(&next_packet, 0, sizeof(next_packet));
	next_packet.id = FLIGHT_COMPUTER_ID; // TODO: change id if ground station
	next_packet.tx_time = (uint64_t)tv_now.tv_sec * 1000000ULL + tv_now.tv_usec;
	next_packet.number = order_number;
	xSemaphoreGive(next_packet_mutex);
}


LoRaPayload* lora_get_tx_packet(void)
{
	xSemaphoreTake(next_packet_mutex, portMAX_DELAY);
	return &next_packet;
}


void lora_release_tx_packet(void)
{
	xSemaphoreGive(next_packet_mutex);
}


uint32_t lora_compute_dt(LoRaPayload *p, uint64_t time)
{
	return time - p->tx_time;
}


bool lora_is_channel_free(void)
{
	int16_t state = radio.scanChannel();
	return state == RADIOLIB_CHANNEL_FREE;
}


void lora_start_receive(uint32_t timeout_ms)
{
	rx_operation_done = false;
	radio.startReceive((uint32_t)timeout_ms * 1000);
}


bool lora_is_reception_done(void)
{
	if (is_tx == true) {
		return false;
	}

	if (rx_operation_done) {
		size_t len = sizeof(LoRaPayload);
		int rx_state = RADIOLIB_ERR_NONE;

		radio.finishReceive();

		rx_state = radio.readData(rx_buffer, len);
		if (rx_state != RADIOLIB_ERR_NONE) {
			// FIXME: log state
			log(S_LORA, T_SYSLOG, "[ERR] receive failed");
		}

		return true;
	}

	return false;
}


LoRaPayload lora_get_rx_packet(void)
{
	LoRaPayload packet;
	memcpy(&packet, rx_buffer, sizeof(packet));
	return packet;
}
