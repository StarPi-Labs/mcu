#include <stdio.h>
#include <RadioLib.h>
#include "PiHal.h"

#include "lora.h"


int main(void)
{

	lora_setup(BAND_L, TX_FORCE, LORA_GS_ID, true);
	LoRaFCState state;

	while (true) {
		state = lora_gs_state_machine();
		printf("GS state: %d\n", state);
	}

	return 0;
}