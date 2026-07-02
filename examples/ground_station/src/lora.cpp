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
	radio.finishTransmit();
}

static void rx_operation_done_cb(void)
{
	rx_operation_done = true;
	radio.finishReceive();
}


void lora_setup(FrequencyBands band, LoRaTxMode mode)
{
	int state = radio.begin();
	if (state != RADIOLIB_ERR_NONE) {
		ERR("failed to initialize radio, code %d", state);
		while (true);
	}

	freq_band = band;
	tx_mode = mode;

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

	LOG("Expected LoRa Time-on-Air %lums", (uint32_t)(radio.getTimeOnAir(sizeof(LoRaPayload))/1000));

	INIT_STATIC_SEMAPHORE(next_packet_mutex);
	if (next_packet_mutex == NULL) {
		ERR("failed to create next_packet mutex");
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


void lora_start_transmission()
{
	static LoRaPayload tx_packet;

	xSemaphoreTake(next_packet_mutex, portMAX_DELAY);
	memcpy(&tx_packet, &next_packet, sizeof(tx_packet));
	xSemaphoreGive(next_packet_mutex);

	switch (tx_mode) {
	case TX_FORCE:
		tx_operation_done = false;
		radio.startTransmit((uint8_t*)&tx_packet, sizeof(tx_packet));
		break;
	case TX_DUTY:
	case TX_POLITE:
		ERR("[LoRa]: TX_DUTY and TX_POLITE modes are not yet implemented");
		// TODO
		break;
	default:
		ERR("[LoRa]: tx mode %d is not valid", tx_mode);
		break;
	}
}


bool lora_is_transmission_done(void)
{
	if (is_tx == false) {
		return false;
	}

	return tx_operation_done;
}


void lora_prepare_next_packet(uint8_t order_number)
{
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	xSemaphoreTake(next_packet_mutex, portMAX_DELAY);
	memset(&next_packet, 0, sizeof(next_packet));
	next_packet.sensor_data.timestamp = (uint64_t)tv_now.tv_sec * 1000000ULL + tv_now.tv_usec;
	next_packet.number = order_number;
	xSemaphoreGive(next_packet_mutex);
}


void lora_update_imu_data(uint64_t time, float altitude, float vspeed, float attitude)
{
	// TODO: verify valid packet
	xSemaphoreTake(next_packet_mutex, portMAX_DELAY);
	next_packet.sensor_data.imu.altitude = altitude;
	next_packet.sensor_data.imu.vspeed = vspeed;
	next_packet.sensor_data.imu.attitude = attitude;
	next_packet.sensor_data.imu.dt = next_packet.sensor_data.timestamp - time;
	xSemaphoreGive(next_packet_mutex);
}


void lora_update_baro_data(uint64_t time, float alt1, float alt2)
{
	xSemaphoreTake(next_packet_mutex, portMAX_DELAY);
	next_packet.sensor_data.baro.alt1 = alt1;
	next_packet.sensor_data.baro.alt2 = alt2;
	next_packet.sensor_data.baro.dt = next_packet.sensor_data.timestamp - time;
	xSemaphoreGive(next_packet_mutex);
}


void lora_update_gps_data(uint64_t time, float latitude, float longitude)
{
	xSemaphoreTake(next_packet_mutex, portMAX_DELAY);
	next_packet.sensor_data.gps.latitude = latitude;
	next_packet.sensor_data.gps.longitude = longitude;
	xSemaphoreGive(next_packet_mutex);
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

		rx_state = radio.readData(rx_buffer, len);
		if (rx_state != RADIOLIB_ERR_NONE) {
			ERR("receive failed: %d", rx_state);
		}

		return true;
	}

	return false;
}


LoRaPayload lora_get_packet(void)
{
	LoRaPayload packet;
	memcpy(&packet, rx_buffer, sizeof(packet));
	return packet;
}
