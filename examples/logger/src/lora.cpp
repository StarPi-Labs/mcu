#include <Arduino.h>
#include <RadioLib.h>

#include "logger.h"
#include "lora.h"
#include "board.h"


extern SPIClass SPI2;
Module radio_module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY, SPI2); //nello stack anziché heap
SX1262 radio(&radio_module);

volatile bool operation_done = false;
volatile int tx_state = RADIOLIB_ERR_NONE;


// As per EN 300 220-2 V3.3.1; Annex B table 1
// https://www.etsi.org/deliver/etsi_en/300200_300299/30022002/03.03.01_60/en_30022002v030301p.pdf
const struct BandRequirements bands[] = {
	[BAND_K] = {.freq_start_mhz = 863.0, .ch_bw_khz = 500, .num_channels = 4, .max_power_mw = 25,  .max_duty = 0.1, .polite_access = true},
	[BAND_L] = {.freq_start_mhz = 865.0, .ch_bw_khz = 500, .num_channels = 6, .max_power_mw = 25,  .max_duty = 1,   .polite_access = true},
	[BAND_M] = {.freq_start_mhz = 868.0, .ch_bw_khz = 500, .num_channels = 1, .max_power_mw = 25,  .max_duty = 1,   .polite_access = true},
	[BAND_N] = {.freq_start_mhz = 868.7, .ch_bw_khz = 500, .num_channels = 1, .max_power_mw = 25,  .max_duty = 0.1, .polite_access = true},
	[BAND_O] = {.freq_start_mhz = 869.4, .ch_bw_khz = 250, .num_channels = 1, .max_power_mw = 500, .max_duty = 10,  .polite_access = true},
};


static void operation_done_cb(void)
{
	operation_done = true;
}


void lora_setup(FrequencyBands band)
{
	int state = radio.begin();
	if (state != RADIOLIB_ERR_NONE) {
		ERR("failed to initialize radio, code %d", state);
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

	radio.setPacketSentAction(operation_done_cb);

	LOG("Expected LoRa Time-on-Air %lums", (uint32_t)(radio.getTimeOnAir(sizeof(LoRaPayload))/1000));

	//primo pacchetto
	//start_transmission(LoRaPayload{0}.bytes, sizeof(LoRaPayload));

	operation_done = true; // Abilita la trasmissione del primo pacchetto
}


void lora_start_transmission(uint8_t* data, size_t size)
{
	if (data == NULL || size == 0) {
		ERR("Invalid data or size for LoRa transmission");
		return;
	}
	if (size > sizeof(LoRaPayload)) {
		ERR("LoRa payload size exceeds maximum");
		return;
	}
	// TODO: check if the radio is busy and return an error if it is, for now we just assume it is always ready
	operation_done = false;
	tx_state = radio.startTransmit(data, size);
}


bool lora_is_transmission_done(void)
{
	if (operation_done) {
		if (tx_state == RADIOLIB_ERR_NONE) {
			//Serial.println(F("transmission finished!"));
		} else {
			ERR("transmission failed: %d", tx_state);
		}

		radio.finishTransmit();
		return true;
	}

	return false;
}
