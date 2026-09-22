#include <stdio.h>
#include <RadioLib.h>
#include "PiHal.h"

#include "lora.h"

#include <unistd.h>
#include <time.h>

void sleep_ms(int milliseconds) {
	struct timespec ts;
	ts.tv_sec = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000L;
	nanosleep(&ts, NULL);
}

int main(void)
{

	lora_setup(BAND_L, TX_FORCE, LORA_GS_ID, true);
	LoRaFCState state;

	while (true) {
		state = lora_gs_state_machine();
		const char *str;
		switch(state) {
		case STATE_DISCONNECTED:
			str = "STATE_DISCONNECTED";
			break;
		case STATE_CONNECTING:
			str = "STATE_CONNECTING";
			break;
		case STATE_TRANSMIT:
			str = "STATE_TRANSMIT";
			break;
		case STATE_RECEIVE:
			str = "STATE_RECEIVE";
			break;
		default:
			str = "UNKNOWN";
			break;
		}
		printf("GS state: %s\n", str);
		sleep_ms(1);
	}

	return 0;
}
