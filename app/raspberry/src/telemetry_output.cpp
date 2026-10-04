#include "telemetry_output.h"

#include <nlohmann/json.hpp>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <chrono>
#include <cmath>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <iostream>

using json = nlohmann::json;

// Lines waiting for the writer thread; the oldest are dropped beyond this
#define TELEMETRY_MAX_QUEUE   256
#define TELEMETRY_MAX_CLIENTS 8

// Global instance
static TelemetryOutput* g_telemetry_output = nullptr;

// Holds SIGINT/SIGTERM back while it is alive. The signal handler in main.cpp
// stops the writer thread, which could never finish if the signal had
// interrupted a thread that holds the queue mutex.
struct SignalBlock {
	sigset_t previous;

	SignalBlock()
	{
		sigset_t signals;
		sigemptyset(&signals);
		sigaddset(&signals, SIGINT);
		sigaddset(&signals, SIGTERM);
		pthread_sigmask(SIG_BLOCK, &signals, &previous);
	}

	~SignalBlock()
	{
		pthread_sigmask(SIG_SETMASK, &previous, NULL);
	}
};


TelemetryOutput::TelemetryOutput(const std::string& socket_path)
	: socket_path_(socket_path) {}


TelemetryOutput::~TelemetryOutput()
{
	stop();
}


bool TelemetryOutput::start()
{
	if (running_.load()) return true;

	server_fd_ = setup_socket();
	if (server_fd_ < 0) {
		std::cerr << "[TelemetryOutput] Failed to setup socket" << std::endl;
		return false;
	}

	running_.store(true);
	writer_thread_ = std::thread(&TelemetryOutput::run, this);
	std::cout << "[TelemetryOutput] Started listening on " << socket_path_ << std::endl;
	return true;
}


void TelemetryOutput::stop()
{
	if (!running_.load()) return;

	running_.store(false);
	cv_.notify_all();

	if (writer_thread_.joinable()) {
		writer_thread_.join();
	}

	for (int client_fd : clients_) {
		close(client_fd);
	}
	clients_.clear();

	if (server_fd_ >= 0) {
		close(server_fd_);
		server_fd_ = -1;
	}

	// Remove socket file
	unlink(socket_path_.c_str());
	std::cout << "[TelemetryOutput] Stopped" << std::endl;
}


int TelemetryOutput::setup_socket()
{
	struct sockaddr_un addr;
	if (socket_path_.size() >= sizeof(addr.sun_path)) {
		std::cerr << "[TelemetryOutput] Socket path too long: " << socket_path_ << std::endl;
		return -1;
	}

	// Remove any existing socket file
	unlink(socket_path_.c_str());

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("[TelemetryOutput] socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_path_.c_str(), sizeof(addr.sun_path) - 1);

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("[TelemetryOutput] bind");
		close(fd);
		return -1;
	}

	if (listen(fd, 5) < 0) {
		perror("[TelemetryOutput] listen");
		close(fd);
		return -1;
	}

	// Non-blocking, the writer thread polls it between lines
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	return fd;
}


void TelemetryOutput::publish(const std::string& line)
{
	{
		SignalBlock block;
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.size() >= TELEMETRY_MAX_QUEUE) {
			queue_.pop_front();
		}
		queue_.push_back(line);
	}
	cv_.notify_one();
}


void TelemetryOutput::publish_state(const std::string& line)
{
	{
		SignalBlock block;
		std::lock_guard<std::mutex> lock(mutex_);
		state_line_ = line;
	}
	publish(line);
}


void TelemetryOutput::run()
{
	// Leave SIGINT/SIGTERM to the other threads: the handler joins this one
	SignalBlock block;

	while (running_.load()) {
		std::deque<std::string> lines;
		{
			// Wake up for a new line, or every 20ms to look for new clients
			std::unique_lock<std::mutex> lock(mutex_);
			cv_.wait_for(lock, std::chrono::milliseconds(20), [this] { return !queue_.empty() || !running_.load(); });
			lines.swap(queue_);
		}

		// Drop the clients that went away (what they send is discarded)
		for (size_t i = 0; i < clients_.size();) {
			char discard[256];
			ssize_t bytes = recv(clients_[i], discard, sizeof(discard), MSG_DONTWAIT);
			if (bytes == 0 || (bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
				close(clients_[i]);
				clients_.erase(clients_.begin() + i);
				std::cout << "[TelemetryOutput] Client disconnected" << std::endl;
			} else {
				i++;
			}
		}

		// Clients that connect now already get the current state from
		// accept_clients(), the queued lines only go to the ones before them
		size_t receivers = clients_.size();
		accept_clients();

		for (const std::string& line : lines) {
			for (size_t i = 0; i < receivers;) {
				if (send_line(clients_[i], line)) {
					i++;
				} else {
					std::cerr << "[TelemetryOutput] Client too slow or gone, dropping it" << std::endl;
					close(clients_[i]);
					clients_.erase(clients_.begin() + i);
					receivers--;
				}
			}
		}
	}
}


void TelemetryOutput::accept_clients()
{
	while (true) {
		int client_fd = accept(server_fd_, NULL, NULL);
		if (client_fd < 0) {
			if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
				perror("[TelemetryOutput] accept");
			}
			return;
		}

		if (clients_.size() >= TELEMETRY_MAX_CLIENTS) {
			std::cerr << "[TelemetryOutput] Too many clients, refusing connection" << std::endl;
			close(client_fd);
			continue;
		}

		std::string state_line;
		{
			std::lock_guard<std::mutex> lock(mutex_);
			state_line = state_line_;
		}
		if (!state_line.empty() && !send_line(client_fd, state_line)) {
			close(client_fd);
			continue;
		}

		clients_.push_back(client_fd);
		std::cout << "[TelemetryOutput] Client connected" << std::endl;
	}
}


bool TelemetryOutput::send_line(int client_fd, const std::string& line)
{
	std::string data = line + "\n";
	// Never wait for the client: if its socket buffer cannot take the whole
	// line it is not reading, and a half-written line cannot be resumed
	ssize_t sent = send(client_fd, data.data(), data.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
	return sent == (ssize_t)data.size();
}


bool telemetry_output_init(const std::string& socket_path)
{
	if (g_telemetry_output != nullptr) {
		std::cerr << "[TelemetryOutput] Already initialized" << std::endl;
		return false;
	}

	g_telemetry_output = new TelemetryOutput(socket_path);
	if (!g_telemetry_output->start()) {
		delete g_telemetry_output;
		g_telemetry_output = nullptr;
		return false;
	}
	return true;
}


void telemetry_output_cleanup()
{
	if (g_telemetry_output) {
		delete g_telemetry_output;
		g_telemetry_output = nullptr;
	}
}


void telemetry_publish_state(LoRaProtoState state)
{
	if (g_telemetry_output == nullptr) return;

	const char *name;
	switch (state) {
	case STATE_DISCONNECTED:
		name = "disconnected";
		break;
	case STATE_CONNECTING:
		name = "connecting";
		break;
	case STATE_TRANSMIT:
		name = "transmit";
		break;
	case STATE_RECEIVE:
		name = "receive";
		break;
	default:
		name = "unknown";
		break;
	}

	json j;
	j["type"] = "state";
	j["state"] = name;
	j["connected"] = (state == STATE_TRANSMIT || state == STATE_RECEIVE);
	g_telemetry_output->publish_state(j.dump());
}


// float16 field of a LoRaDataPacket, as JSON: null if it is not a finite number
static json half_to_json(uint16_t bits)
{
	float f = half_float::half_cast<float>(std::bit_cast<half_float::half>(bits));
	return std::isfinite(f) ? json(f) : json(nullptr);
}


static json float_to_json(float f)
{
	return std::isfinite(f) ? json(f) : json(nullptr);
}


void telemetry_publish_data(const LoRaDataPacket& packet)
{
	if (g_telemetry_output == nullptr) return;

	// The packet is packed: copy the fields out instead of binding references
	LoRaPacketHeader header = packet.header;

	json j;
	j["type"] = "data";
	j["id"] = header.id;
	j["number"] = header.number;
	j["tx_time_ms"] = u48le_to_u64(header.tx_time);
	j["state"] = (uint8_t)packet.state;
	j["imu"] = {
		{"altitude", half_to_json(packet.imu.altitude)},
		{"vspeed", half_to_json(packet.imu.vspeed)},
		{"attitude", half_to_json(packet.imu.attitude)},
		{"dt_ms", (int16_t)packet.imu.dt},
	};
	j["baro"] = {
		{"p1", half_to_json(packet.baro.p1)},
		{"p2", half_to_json(packet.baro.p2)},
		{"dt_ms", (int16_t)packet.baro.dt},
	};
	j["gps"] = {
		{"latitude", float_to_json(packet.gps.latitude)},
		{"longitude", float_to_json(packet.gps.longitude)},
		{"dt_ms", (int16_t)packet.gps.dt},
	};
	// How well the ground station hears the flight computer
	j["rssi_dbm"] = float_to_json(lora_get_rssi());
	j["median_bps"] = float_to_json(lora_get_median_bps());
	g_telemetry_output->publish(j.dump());
}
