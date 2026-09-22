#include <stdio.h>
#include <RadioLib.h>
#include "PiHal.h"

#include "../../include/lora.h"


#define DIO1_PIN  18
#define RESET_PIN 23
#define BUSY_PIN  25
#define NSS_PIN   8
#define MISO_PIN  9
#define MOSI_PIN  10
#define SCK_PIN   11


PiHal* hal = new PiHal(0);
SX1262 radio = new Module(
	hal,
	NSS_PIN,
	DIO1_PIN,
	RESET_PIN,
	BUSY_PIN
);

int main(void)
{

	int state = radio.begin();

	if(state != RADIOLIB_ERR_NONE) {
		printf("Radio init failed: %d\n", state);
		return 1;
	}

	while (true) {
		printf("Hello World\n");
		hal->delay(1000);
	}

	return 0;
}