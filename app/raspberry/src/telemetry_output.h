#pragma once

#include <thread>
#include <atomic>
#include <string>
#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "lora.h"

// Telemetry output - the machine-readable counterpart of command_input.
//
// A Unix domain socket (default /tmp/starpi_tlm.sock) on which every connected
// client receives JSON lines (one object per line, '\n' terminated). Clients
// only read; anything they send is ignored.
//
// 1. Protocol state, sent to a client as soon as it connects and then on every
//    change of the ground station state machine:
//
//      {"type":"state","state":"receive","connected":true}
//
//    state:     "disconnected" | "connecting" | "transmit" | "receive"
//    connected: true once the handshake with the flight computer is done
//               (transmit/receive). It goes back to false when the flight
//               computer has been silent for MAX_SILENT_FRAMES frames.
//
// 2. One line per LoRaDataPacket received from the flight computer:
//
//      {"type":"data","id":252,"number":17,"tx_time_ms":1789583292713,"state":2,
//       "imu":{"altitude":812.5,"vspeed":95.25,"attitude":3.5,"dt_ms":-12},
//       "baro":{"p1":921.5,"p2":921.0,"dt_ms":-31},
//       "gps":{"latitude":39.392547,"longitude":-8.289517,"dt_ms":-180},
//       "rssi_dbm":-87.5,"median_bps":2140.0}
//
//    id, number: sender id and sequence number from the packet header (the
//                number counts every packet the flight computer transmits,
//                modulo 256, so a gap is lost packets)
//    tx_time_ms: flight computer clock when the packet was transmitted, in
//                milliseconds (epoch once its GPS has set the clock)
//    state:      RocketState ordinal (logger.h)
//    altitude m, vspeed m/s, attitude degrees from vertical, p1/p2 mbar,
//    latitude/longitude degrees: the float16 fields already converted. A value
//    that is not finite is sent as null.
//    dt_ms:      when the group was last updated, relative to tx_time_ms: the
//                sample was taken at tx_time_ms + dt_ms on the flight computer
//                clock
//    rssi_dbm:   signal strength of this packet at the ground station
//    median_bps: median bit rate of the link over the last packets, both ways
//
// New keys may be added; clients must ignore the ones they do not know.
//
// The radio loop never waits for a client: publish() only queues the line, a
// separate thread writes it out without blocking, and a client that does not
// keep up is disconnected (it can reconnect). With no client the lines are
// dropped.
class TelemetryOutput {
public:
	explicit TelemetryOutput(const std::string& socket_path);

	~TelemetryOutput();

	// Start the socket thread, returns false if the socket cannot be created
	bool start();

	// Stop the socket thread and disconnect the clients
	void stop();

	// Queue a line (without the trailing newline) for every connected client
	void publish(const std::string& line);

	// Same, and keep the line as the one sent first to clients that connect later
	void publish_state(const std::string& line);

private:
	// Main thread function
	void run();

	// Accept the pending connections, greeting each with the current state
	void accept_clients();

	// Send a line to a client without blocking, returns false if it must be dropped
	bool send_line(int client_fd, const std::string& line);

	// Setup and bind the Unix domain socket
	int setup_socket();

	std::string socket_path_;
	std::thread writer_thread_;
	std::atomic<bool> running_{false};
	int server_fd_ = -1;
	std::vector<int> clients_; // only touched by the writer thread

	std::mutex mutex_;
	std::condition_variable cv_;
	std::deque<std::string> queue_;
	std::string state_line_;
};

// Initialize the telemetry output system
bool telemetry_output_init(const std::string& socket_path = "/tmp/starpi_tlm.sock");

// Cleanup the telemetry output system
void telemetry_output_cleanup();

// Publish the protocol state, call it when the state changes
void telemetry_publish_state(LoRaProtoState state);

// Publish a data packet received from the flight computer
void telemetry_publish_data(const LoRaDataPacket& packet);
