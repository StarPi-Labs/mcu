#include <Arduino.h>
#include <RadioLib.h>

#include "Logging.h"
#include "board.h"
#include "lora.h"

extern SPIClass SPI2;
Module radio_module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY,
                    SPI2); // nello stack anziché heap
SX1262 radio(&radio_module);

volatile bool operation_done = false;
int tx_state = RADIOLIB_ERR_NONE;

static void operation_done_cb(void)
{
  operation_done = true;
}

void lora_setup(void)
{
  int state = radio.begin();
  if (state != RADIOLIB_ERR_NONE) {
    mcu_log_error("failed to initialize radio, code {}\n", state);
    radio.setFrequency(LORA_FREQUENCY);
    radio.setOutputPower(LORA_OUTPUT_POWER);
    radio.setBandwidth(LORA_BANDWIDTH);
    radio.setSpreadingFactor(LORA_SPREADING_FACTOR);
    radio.setCodingRate(LORA_CODING_RATE);
    radio.forceLDRO(false);
    radio.implicitHeader(sizeof(LoRaPayload)); // PACCHETTI A LUNGHEZZA FISSA
    radio.setCRC(LORA_CRC_BYTES);

    radio.setPacketSentAction(operation_done_cb);

    mcu_log_info("Expected LoRa Time-on-Air [ms] {}\n",
                 (uint32_t)(radio.getTimeOnAir(sizeof(LoRaPayload)) / 1000));

    // primo pacchetto
    // start_transmission(LoRaPayload{0}.bytes, sizeof(LoRaPayload));

    operation_done = true; // Abilita la trasmissione del primo pacchetto
  }
}

void lora_start_transmission(uint8_t* data, size_t size)
{
  if (data == NULL || size == 0) {
    mcu_log_error("Invalid data or size for LoRa transmission\n");
    return;
  }
  if (size > sizeof(LoRaPayload)) {
    mcu_log_error("LoRa payload size exceeds maximum\n");
    return;
  }
  // TODO: check if the radio is busy and return an error if it is, for now we
  // just assume it is always ready
  operation_done = false;
  tx_state = radio.startTransmit(data, size);
}

bool lora_is_transmission_done(void)
{
  if (operation_done) {
    if (tx_state == RADIOLIB_ERR_NONE) {
      // Serial.println(F("transmission finished!"));
    } else {
      mcu_log_error("transmission failed: {}\n", tx_state);
    }

    radio.finishTransmit();
  }

  return operation_done;
}