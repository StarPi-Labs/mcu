#include "command_input.h"
#include "circular_buffer.h"
#include "lora.h"

#include <nlohmann/json.hpp>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <iostream>

using json = nlohmann::json;

// Global instances
CommandBuffer g_command_buffer;
CommandInput* g_command_input = nullptr;


CommandInput::CommandInput(const std::string& socket_path, CommandBuffer& buffer)
	: socket_path_(socket_path), buffer_(buffer) {}


CommandInput::~CommandInput()
{
	stop();
}


void CommandInput::start()
{
	if (running_.load()) return;

	server_fd_ = setup_socket();
	if (server_fd_ < 0) {
		std::cerr << "[CommandInput] Failed to setup socket" << std::endl;
		return;
	}

	running_.store(true);
	listener_thread_ = std::thread(&CommandInput::run, this);
	std::cout << "[CommandInput] Started listening on " << socket_path_ << std::endl;
}


void CommandInput::stop()
{
	if (!running_.load()) return;

	running_.store(false);

	// Close server socket to unblock accept()
	if (server_fd_ >= 0) {
		close(server_fd_);
		server_fd_ = -1;
	}

	if (listener_thread_.joinable()) {
		listener_thread_.join();
	}

	// Remove socket file
	unlink(socket_path_.c_str());
	std::cout << "[CommandInput] Stopped" << std::endl;
}


int CommandInput::setup_socket()
{
	// Remove any existing socket file
	unlink(socket_path_.c_str());

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("[CommandInput] socket");
		return -1;
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_path_.c_str(), sizeof(addr.sun_path) - 1);

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("[CommandInput] bind");
		close(fd);
		return -1;
	}

	if (listen(fd, 5) < 0) {
		perror("[CommandInput] listen");
		close(fd);
		return -1;
	}

	// Set non-blocking to allow clean shutdown
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	return fd;
}


void CommandInput::run()
{
	while (running_.load()) {
		struct sockaddr_un client_addr;
		socklen_t client_len = sizeof(client_addr);

		int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);

		if (client_fd < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// No connection pending, sleep briefly and retry
				usleep(10000); // 10ms
				continue;
			}
			if (errno == EINTR || errno == EBADF) {
				// Interrupted or socket closed (shutdown)
				break;
			}
			perror("[CommandInput] accept");
			continue;
		}

		// Handle client in this thread (simple sequential handling)
		// For concurrent clients, you'd spawn a thread per client
		handle_client(client_fd);
		close(client_fd);
	}
}


void CommandInput::handle_client(int client_fd)
{
	char buffer[4096];
	std::string accumulated;

	while (running_.load()) {
		ssize_t bytes = read(client_fd, buffer, sizeof(buffer) - 1);

		if (bytes <= 0) {
			// Client disconnected or error
			break;
		}

		buffer[bytes] = '\0';
		accumulated += buffer;

		// Process complete lines (JSON objects terminated by newline)
		size_t pos;
		while ((pos = accumulated.find('\n')) != std::string::npos) {
			std::string line = accumulated.substr(0, pos);
			accumulated.erase(0, pos + 1);

			if (line.empty()) continue;

			LoRaCommandPacket packet;
			if (parse_command(line, packet)) {
				packet.header.id = LORA_GS_ID; // Set the ground station ID
				packet.header.type = PKT_COMMAND; // Set packet type to command
				if (buffer_.push(packet)) {
					std::cout << "[CommandInput] Command queued: cmd=" << static_cast<int>(packet.command)
							  << ", data=" << packet.data << std::endl;
					if (command_callback_) {
						command_callback_(packet);
					}
				} else {
					std::cerr << "[CommandInput] Buffer full, dropping command" << std::endl;
				}
			} else {
				std::cerr << "[CommandInput] Failed to parse command: " << line << std::endl;
			}
		}
	}
}


bool CommandInput::parse_command(const std::string& json_str, LoRaCommandPacket& packet)
{
	try {
		json j = json::parse(json_str);

		// Initialize packet with zeros
		memset(&packet, 0, sizeof(packet));

		// Parse command
		if (j.contains("command")) {
			packet.command = static_cast<LoRaCommand>(j["command"].get<uint8_t>());
		} else {
			std::cerr << "[CommandInput] Missing required field: command" << std::endl;
			return false;
		}

		// Parse data (optional, defaults to 0)
		if (j.contains("data")) {
			packet.data = j["data"].get<uint64_t>();
		} else {
			packet.data = 0;
		}

		return true;
	} catch (const json::exception& e) {
		std::cerr << "[CommandInput] JSON parse error: " << e.what() << std::endl;
		return false;
	} catch (const std::exception& e) {
		std::cerr << "[CommandInput] Parse error: " << e.what() << std::endl;
		return false;
	}
}


bool command_input_init(const std::string& socket_path)
{
	if (g_command_input != nullptr) {
		std::cerr << "[CommandInput] Already initialized" << std::endl;
		return false;
	}

	g_command_input = new CommandInput(socket_path, g_command_buffer);
	g_command_input->start();
	return true;
}


void command_input_cleanup()
{
	if (g_command_input) {
		delete g_command_input;
		g_command_input = nullptr;
	}
}