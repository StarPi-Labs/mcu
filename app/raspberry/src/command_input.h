#pragma once

#include <thread>
#include <atomic>
#include <string>
#include <functional>

#include "circular_buffer.h"
#include "lora.h"

// Command input handler - reads JSON commands from Unix domain socket
// and pushes parsed LoRaCommandPackets into a circular buffer
class CommandInput {
public:
	// Constructor takes the socket path and a reference to the command buffer
	CommandInput(const std::string& socket_path, CommandBuffer& buffer);

	~CommandInput();

	// Start the socket listener thread
	void start();

	// Stop the socket listener thread
	void stop();

	// Check if running
	bool is_running() const { return running_.load(); }

	// Set callback for when a command is successfully parsed (optional)
	void set_command_callback(std::function<void(const LoRaCommandPacket&)> callback) {
		command_callback_ = callback;
	}

private:
	// Main thread function
	void run();

	// Handle a client connection
	void handle_client(int client_fd);

	// Parse JSON string into LoRaCommandPacket
	bool parse_command(const std::string& json_str, LoRaCommandPacket& packet);

	// Setup and bind the Unix domain socket
	int setup_socket();

	std::string socket_path_;
	CommandBuffer& buffer_;
	std::thread listener_thread_;
	std::atomic<bool> running_{false};
	int server_fd_ = -1;
	std::function<void(const LoRaCommandPacket&)> command_callback_;
};

// Global command buffer instance (defined in command_input.cpp)
extern CommandBuffer g_command_buffer;

// Global command input instance
extern CommandInput* g_command_input;

// Initialize the command input system
bool command_input_init(const std::string& socket_path = "/tmp/starpi_cmd.sock");

// Cleanup the command input system
void command_input_cleanup();