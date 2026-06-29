#include <Arduino.h>
#include <RadioLib.h>

#include "lora.h"
#include "board.h"


extern SPIClass SPI2;
Module radio_module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY, SPI2); //nello stack anziché heap
SX1262 radio(&radio_module);

static volatile bool tx_operation_done = false;
static volatile bool rx_operation_done = false;
static volatile int tx_state = RADIOLIB_ERR_NONE;
static volatile int rx_state = RADIOLIB_ERR_NONE;

static uint8_t rx_buffer[sizeof(LoRaPayload)];
LoRaPayload next_packet;


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



void lora_setup(FrequencyBands band)
{
	int state = radio.begin();
	if (state != RADIOLIB_ERR_NONE) {
		Serial.printf("failed to initialize radio, code %d\n", state);
		while (true);
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

	// TODO: DIO1 cannot be set as both the transmission done and receprion done
	//        interrupt so we need to implement a mode switch behavior which
	//         reassigns the interrupt pin with a different callback
	//radio.setPacketSentAction(tx_operation_done_cb);
	radio.setPacketReceivedAction(rx_operation_done_cb);

	Serial.printf("Expected LoRa Time-on-Air %lums\n", (uint32_t)(radio.getTimeOnAir(sizeof(LoRaPayload))/1000));

	//primo pacchetto
	//start_transmission(LoRaPayload{0}.bytes, sizeof(LoRaPayload));

	// Abilita la trasmissione e ricezione del primo pacchetto
	tx_operation_done = true;
	rx_operation_done = true;
	lora_prepare_next_packet(0);
}


void lora_start_transmission()
{
	static LoRaPayload tx_packet;

	// TODO: add a mutex to the packet
	memcpy(&tx_packet, &next_packet, sizeof(tx_packet));
	// TODO: check if the radio is busy and return an error if it is, for now we just assume it is always ready
	tx_operation_done = false;
	tx_state = radio.startTransmit((uint8_t*)&tx_packet, sizeof(tx_packet));
}


bool lora_is_transmission_done(void)
{
	if (tx_operation_done) {
		if (tx_state == RADIOLIB_ERR_NONE) {
			//Serial.println(F("transmission finished!"));
		} else {
			Serial.printf("transmission failed: %d\n", tx_state);
		}

		radio.finishTransmit();
		return true;
	}

	return false;
}


void lora_prepare_next_packet(uint8_t order_number)
{
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	memset(&next_packet, 0, sizeof(next_packet));
	next_packet.sensor_data.timestamp = (uint64_t)tv_now.tv_sec * 1000000ULL + tv_now.tv_usec;
	next_packet.number = order_number;
}


void lora_update_imu_data(uint64_t time, float altitude, float vspeed, float attitude)
{
	// TODO: verify valid packet
	next_packet.sensor_data.imu.altitude = altitude;
	next_packet.sensor_data.imu.vspeed = vspeed;
	next_packet.sensor_data.imu.attitude = attitude;
	next_packet.sensor_data.imu.dt = next_packet.sensor_data.timestamp - time;
}


void lora_update_baro_data(uint64_t time, float alt1, float alt2)
{
	next_packet.sensor_data.baro.alt1 = alt1;
	next_packet.sensor_data.baro.alt2 = alt2;
	next_packet.sensor_data.baro.dt = next_packet.sensor_data.timestamp - time;
}


void lora_update_gps_data(uint64_t time, float latitude, float longitude)
{
	next_packet.sensor_data.gps.latitude = latitude;
	next_packet.sensor_data.gps.longitude = longitude;
}


bool lora_is_channel_free(void)
{
	int16_t state = radio.scanChannel();
	return state == RADIOLIB_CHANNEL_FREE;
}


void lora_start_receive(uint32_t timeout_ms)
{
	rx_operation_done = false;
	rx_state = radio.startReceive((uint32_t)timeout_ms * 1000);
}


bool lora_is_reception_done(void)
{
	if (rx_operation_done) {
		if (rx_state == RADIOLIB_ERR_NONE) {
			size_t len = sizeof(LoRaPayload);
			rx_state = radio.readData(rx_buffer, len);
			if (rx_state != RADIOLIB_ERR_NONE) {
				Serial.printf("receive failed: %d\n", rx_state);
			}
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