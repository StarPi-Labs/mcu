#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

#include "logger.h"


// Logger implementation, this is a simple wrapper around a FreeRTOS queue.
// This is a simple wrapper around a FreeRTOS queue. The messages are stored in
// a statically allocated buffer, so there is no dynamic memory allocation.
// The logger is designed to be used in a producer-consumer pattern.


// Buffers for the message queues
// NOTE: these could be in PSRAM if we had it
// UART queue
static uint8_t uart_queue_buffer[MESSAGE_QUEUE_SIZE * sizeof(message_t)];
static StaticQueue_t uart_queue_desc;
static QueueHandle_t uart_queue;
// SD card queue
static uint8_t sd_queue_buffer[MESSAGE_QUEUE_SIZE * sizeof(message_t)];
static StaticQueue_t sd_queue_desc;
static QueueHandle_t sd_queue;
// LoRa queue
static uint8_t lora_queue_buffer[MESSAGE_QUEUE_SIZE * sizeof(message_t)];
static StaticQueue_t lora_queue_desc;
static QueueHandle_t lora_queue;


// initialize the default message queue, this should be called before using any
// of the other message queue functions
bool message_queue_init()
{
	uart_queue = xQueueCreateStatic(MESSAGE_QUEUE_SIZE, sizeof(message_t), uart_queue_buffer, &uart_queue_desc);
	if (uart_queue == NULL) return false;
	sd_queue = xQueueCreateStatic(MESSAGE_QUEUE_SIZE, sizeof(message_t), sd_queue_buffer, &sd_queue_desc);
	if (sd_queue == NULL) return false;
	lora_queue = xQueueCreateStatic(MESSAGE_QUEUE_SIZE, sizeof(message_t), lora_queue_buffer, &lora_queue_desc);
	if (lora_queue == NULL) return false;
	
	return true;
}

bool message_queue_full(message_dest_t dest)
{
	switch(dest) {
	case DEST_UART:
		return uxQueueSpacesAvailable(uart_queue) <= 0;
		break;
	case DEST_SD:
		return uxQueueSpacesAvailable(sd_queue) <= 0;
		break;
	case DEST_LORA:
		return uxQueueSpacesAvailable(lora_queue) <= 0;
		break;
	default:
		return false;
	}
}

// reset the message queue, this should be called with caution as it will discard
// all messages in the queue
bool message_queue_reset(message_dest_t dest)
{
	switch(dest) {
	case DEST_UART:
		xQueueReset(uart_queue);
		break;
	case DEST_SD:
		xQueueReset(sd_queue);
		break;
	case DEST_LORA:
		xQueueReset(lora_queue);
		break;
	case DEST_ALL:
		xQueueReset(uart_queue);
		xQueueReset(sd_queue);
		xQueueReset(lora_queue);
		break;
	default:
		break;
	}

	return true;
}


// push the message to the back of the queue, this should not be called from an
// ISR context
bool message_queue_enqueue(message_t *message, TickType_t timeout)
{
	if (message == NULL) return false;
	bool err = false;

	if (message->dest & DEST_UART) {
		err |= xQueueSend(uart_queue, message, timeout) != pdPASS;
	}

	if (message->dest & DEST_SD) {
		err |= xQueueSend(sd_queue, message, timeout) != pdPASS;
	}

	if (message->dest & DEST_LORA) {
		err |= xQueueSend(lora_queue, message, timeout) != pdPASS;
	}

	return !err;
}


// pop the message from the front of the queue, this should not be called from
// an ISR context
bool message_queue_dequeue(message_t *message, TickType_t timeout, message_dest_t dest)
{
	if (dest == DEST_ALL || message == NULL) return false;
	
	QueueHandle_t handle = NULL;
	switch (dest) {
	case DEST_UART:
		handle = uart_queue;	
		break;
	case DEST_SD:
		handle = sd_queue;
		break;
	case DEST_LORA:
		handle = lora_queue;
		break;
	case DEST_NONE:
	default:
		return true;
		break;
	}
	if (xQueueReceive(handle, message, timeout) != pdPASS) return false;
	return true;
}


// peek at the message at the front of the queue without removing it, this should
// not be called from an ISR context
bool message_queue_peek(message_t *message, TickType_t timeout, message_dest_t dest)
{
	if (dest == DEST_ALL || message == NULL) return false;
	
	QueueHandle_t handle = NULL;
	switch (dest) {
	case DEST_UART:
		handle = uart_queue;	
		break;
	case DEST_SD:
		handle = sd_queue;
		break;
	case DEST_LORA:
		handle = lora_queue;
		break;
	case DEST_NONE:
	default:
		return true;
		break;
	}
	if (xQueuePeek(handle, message, timeout) != pdPASS) return false;
	return true;
}


/**
 * @brief Formats a message_t into a provided string buffer.
 * * @param msg  Pointer to the message_t structure.
 * @param buf  Pointer to the destination character buffer.
 * @param size Size of the destination buffer.
 * @return The number of characters written (excluding null byte).
 */
int format_message_to_string(const message_t *msg, char *buf, size_t size) {
	if (!msg || !buf || size == 0) return 0;

	int written = 0;
	const char *desc = msg->description ? msg->description : "no_desc";

#ifdef CONFIG_USE_HUMAN_READABLE_TIMESTAMPS
	// Split microseconds into seconds and fractional microseconds
	time_t seconds = (time_t)(msg->timestamp / 1000000ULL);
	uint32_t microseconds = (uint32_t)(msg->timestamp % 1000000ULL);

	// localtime_r is thread-safe, which is critical in ESP-IDF (FreeRTOS)
	struct tm timeinfo;
	localtime_r(&seconds, &timeinfo);

	char time_str[24];
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);

	// Format: [YYYY-MM-DD HH:MM:SS.uuuuuu] description:
	written = snprintf(buf, size, "[%s.%06" PRIu32 "] %s", time_str, microseconds, desc);
#else
	// Format: [00000000000000000000] description:
	// Uses PRIu64 to safely format uint64_t across 32-bit and 64-bit platforms
	written = snprintf(buf, size, "[%020" PRIu64 "] %s", msg->timestamp, desc);
#endif

	// Prevent buffer overflows on the remaining payload
	if (written < 0 || (size_t)written >= size) {
		return written;
	}

	char *ptr = buf + written;
	size_t rem = size - written;

	// Append the union data based on the type
	switch (msg->type) {
	case MSG_NONE:
		return written;
	case MSG_INT32:
		return written + snprintf(ptr, rem, ": %" PRId32, msg->data.i32);
	case MSG_UINT32:
		return written + snprintf(ptr, rem, ": %" PRIu32, msg->data.u32);
	case MSG_INT64:
		return written + snprintf(ptr, rem, ": %" PRId64, msg->data.i64);
	case MSG_UINT64:
		return written + snprintf(ptr, rem, ": %" PRIu64, msg->data.u64);
	case MSG_FLOAT:
		return written + snprintf(ptr, rem, ": %.4g", msg->data.f);
	case MSG_DOUBLE:
		return written + snprintf(ptr, rem, ": %.6g", msg->data.d);
	case MSG_STRING:
		return written + snprintf(ptr, rem, ": %s", msg->data.str ? msg->data.str : "NULL");
	case MSG_VEC3:
		return written + snprintf(ptr, rem, ": (%.3g, %.3g, %.3g)", msg->data.v3.x, msg->data.v3.y, msg->data.v3.z);
	case MSG_IVEC3:
		return written + snprintf(ptr, rem, ": (%" PRId32 ", %" PRId32 ", %" PRId32 ")", msg->data.iv3.x, msg->data.iv3.y, msg->data.iv3.z);
	default:
		return written + snprintf(ptr, rem, ": UNKNOWN_TYPE");
	}
}
