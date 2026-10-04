#include <stdio.h>
#include <RadioLib.h>
#include "PiHal.h"

#include "lora.h"
#include "command_input.h"

#include <unistd.h>
#include <time.h>
#include <signal.h>

void sleep_ms(int milliseconds) {
	struct timespec ts;
	ts.tv_sec = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000L;
	nanosleep(&ts, NULL);
}


LoRaDataPacket rx_packet;

// Callback to fetch next command packet from circular buffer
bool tx_packet_cb(uint8_t* packet)
{
	LoRaCommandPacket cmd;
	if (g_command_buffer.pop(cmd)) {
		*((LoRaCommandPacket*)packet) = cmd;
		return true;
	}
	return false; // No packet available
}


void rx_packet_cb(uint8_t* packet)
{
	rx_packet = *((LoRaDataPacket*)packet);
}


// Signal handler for clean shutdown
static void signal_handler(int sig) {
	printf("\n[Main] Received signal %d, shutting down...\n", sig);
	command_input_cleanup();
	exit(0);
}

int main(void)
{
	// Setup signal handlers
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	// Initialize command input (Unix domain socket)
	if (!command_input_init("/tmp/starpi_cmd.sock")) {
		fprintf(stderr, "[Main] Failed to initialize command input\n");
		return 1;
	}

	lora_setup(BAND_L, TX_FORCE, LORA_GS_ID, true);
	lora_set_tx_packet_cb(tx_packet_cb);
	lora_set_rx_packet_cb(rx_packet_cb);

	LoRaProtoState state;

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
		if (state = STATE_DISCONNECTED) {
			printf("GS state: %s\n", str);
		}
		if (state == STATE_RECEIVE) {
			auto alt = std::bit_cast<half_float::half>(rx_packet.imu.altitude);
			auto vsp = std::bit_cast<half_float::half>(rx_packet.imu.vspeed);
			auto att = std::bit_cast<half_float::half>(rx_packet.imu.attitude);
			auto p1 = std::bit_cast<half_float::half>(rx_packet.baro.p1);
			auto p2 = std::bit_cast<half_float::half>(rx_packet.baro.p2);
			printf("Received packet: state=%d, altitude=%f, vspeed=%f, attitude=%f, dt=%d, p1=%f, p2=%f, dt=%d, latitude=%f, longitude=%f, dt=%d, rssi=%.2f\n",
				rx_packet.state,
				half_float::half_cast<float>(alt),
				half_float::half_cast<float>(vsp),
				half_float::half_cast<float>(att),
				rx_packet.imu.dt,
				half_float::half_cast<float>(p1),
				half_float::half_cast<float>(p2),
				rx_packet.baro.dt,
				rx_packet.gps.latitude,
				rx_packet.gps.longitude,
				rx_packet.gps.dt,
				lora_get_rssi()
			);
		}
		//sleep_ms(1);
	}

	return 0;
}
