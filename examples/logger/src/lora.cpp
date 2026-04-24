#include <Arduino.h>
#include <RadioLib.h>

#include "logger.h"
#include "lora.h"
#include "board.h"


extern SPIClass SPI2;
Module radio_module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY, SPI2); //nello stack anziché heap
SX1262 radio(&radio_module);

volatile bool operation_done = false;
int tx_state = RADIOLIB_ERR_NONE;



static void operation_done_cb(void)
{
	operation_done = true;
}


void lora_setup (void)
{
	int state = radio.begin();
	if (state != RADIOLIB_ERR_NONE) {
		LOG("failed to initialize radio, code", state);
		while (true);
	}

	radio.setDio2AsRfSwitch(true); //
	radio.setOutputPower(LORA_OUTPUT_POWER);
	radio.setBandwidth(LORA_BANDWIDTH);
	radio.setSpreadingFactor(LORA_SPREADING_FACTOR);
	radio.setCodingRate(LORA_CODING_RATE);
	radio.forceLDRO(false);
	radio.implicitHeader(sizeof(LoRaPayload)); // PACCHETTI A LUNGHEZZA FISSA
	radio.setCRC(true);

	radio.setPacketSentAction(operation_done_cb);

	//long timeOnAir_us = radio.getTimeOnAir(sizeof(LoRaPayload));
	//float freqHz = 1000000.0 / timeOnAir_us;

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
			ERR("transmission failed", tx_state);
		}

		radio.finishTransmit();
	}

	return operation_done;
}
