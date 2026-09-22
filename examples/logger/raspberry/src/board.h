#pragma once

#define LORA_DIO1 18
#define LORA_RST  23
#define LORA_BUSY 25
#define LORA_CS   8
#define LORA_MISO 9
#define LORA_MOSI 10
#define LORA_SCK  11

// LoRa configuration
#define LORA_OUTPUT_POWER     0     // dBm, da capire
#define LORA_SPREADING_FACTOR 7     // minimo spread factor
#define LORA_CODING_RATE      5     // 4 + 1, 1 bit su 8 di correzione di errore
#define LORA_CRC_BYTES        1

