#include <stdio.h>
#include <RadioLib.h>
#include "PiHal.h"

#include "lora.h"
#include "command_input.h"
#include "telemetry_output.h"

#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string>

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
	// The state machine hands over whatever it received in the FC window
	if (rx_packet.header.type == PKT_DATA && rx_packet.header.id == LORA_FC_ID) {
		telemetry_publish_data(rx_packet);
	}
}


// Signal handler for clean shutdown
static void signal_handler(int sig) {
	// A signal sent to the whole process group can arrive twice, on two threads
	static volatile sig_atomic_t stopping = 0;
	if (stopping) return;
	stopping = 1;

	printf("\n[Main] Received signal %d, shutting down...\n", sig);
	command_input_cleanup();
	telemetry_output_cleanup();
	exit(0);
}

// Usage: radio_app [socket_dir]
// The sockets are <socket_dir>/starpi_cmd.sock (commands in, see command_input.h)
// and <socket_dir>/starpi_tlm.sock (telemetry and state out, see telemetry_output.h).
// socket_dir comes from the first argument, else from STARPI_SOCKET_DIR, else /tmp.
int main(int argc, char **argv)
{
	std::string socket_dir = "/tmp";
	if (argc > 1) {
		socket_dir = argv[1];
	} else if (getenv("STARPI_SOCKET_DIR") != NULL && getenv("STARPI_SOCKET_DIR")[0] != '\0') {
		socket_dir = getenv("STARPI_SOCKET_DIR");
	}
	mkdir(socket_dir.c_str(), 0755); // fine if it already exists, bind() reports the rest

	std::string cmd_socket = socket_dir + "/starpi_cmd.sock";
	std::string tlm_socket = socket_dir + "/starpi_tlm.sock";
	// sizeof(sockaddr_un::sun_path), a longer path would be silently truncated
	if (cmd_socket.size() >= 108 || tlm_socket.size() >= 108) {
		fprintf(stderr, "[Main] Socket directory path too long: %s\n", socket_dir.c_str());
		return 1;
	}

	// Setup signal handlers
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	// Initialize command input (Unix domain socket)
	if (!command_input_init(cmd_socket)) {
		fprintf(stderr, "[Main] Failed to initialize command input\n");
		return 1;
	}

	// Initialize telemetry output (Unix domain socket)
	if (!telemetry_output_init(tlm_socket)) {
		fprintf(stderr, "[Main] Failed to initialize telemetry output\n");
		return 1;
	}
	LoRaProtoState published_state = STATE_DISCONNECTED;
	telemetry_publish_state(published_state);

	lora_setup(BAND_L, TX_FORCE, LORA_GS_ID, true);
	lora_set_tx_packet_cb(tx_packet_cb);
	lora_set_rx_packet_cb(rx_packet_cb);

	LoRaProtoState state, prev_state = STATE_CONNECTING;

	while (true) {
		state = lora_gs_state_machine();
		if (state != published_state) {
			published_state = state;
			telemetry_publish_state(state);
		}
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
		if (state != prev_state) {
			printf("GS state: %s, median_bps: %.1f\n", str, lora_get_median_bps());
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
		prev_state = state;
	}

	return 0;
}
