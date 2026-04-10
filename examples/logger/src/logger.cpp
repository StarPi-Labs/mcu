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


// Buffer for the default message queue
// NOTE: this could be in PSRAM if we had it
static uint8_t default_queue_buffer[MESSAGE_QUEUE_SIZE * sizeof(message_t)];
static StaticQueue_t default_queue_desc;

// default message queue, this is initialized in message_queue_init()
message_queue_t DEFAULT_QUEUE = NULL;


// initialize the default message queue, this should be called before using any
// of the other message queue functions
bool message_queue_init()
{
	DEFAULT_QUEUE = xQueueCreateStatic(MESSAGE_QUEUE_SIZE, sizeof(message_t), default_queue_buffer, &default_queue_desc);
	if (DEFAULT_QUEUE == NULL) return false;
	return true;
}


// reset the message queue, this should be called with caution as it will discard
// all messages in the queue
bool message_queue_reset()
{
	if (DEFAULT_QUEUE == NULL) return false;
	xQueueReset(DEFAULT_QUEUE);
	return true;
}


// push the message to the back of the queue, this should not be called from an
// ISR context
bool message_queue_enqueue(message_t *message, TickType_t timeout)
{
	if (DEFAULT_QUEUE == NULL || message == NULL) return false;
	if (xQueueSend(DEFAULT_QUEUE, message, timeout) != pdPASS) return false;
	return true;
}


// pop the message from the front of the queue, this should not be called from
// an ISR context
bool message_queue_dequeue(message_t *message, TickType_t timeout)
{
	if (DEFAULT_QUEUE == NULL || message == NULL) return false;
	if (xQueueReceive(DEFAULT_QUEUE, message, timeout) != pdPASS) return false;
	return true;
}


// peek at the message at the front of the queue without removing it, this should
// not be called from an ISR context
bool message_queue_peek(message_t *message, TickType_t timeout)
{
	if (DEFAULT_QUEUE == NULL || message == NULL) return false;
	if (xQueuePeek(DEFAULT_QUEUE, message, timeout) != pdPASS) return false;
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
	default:
		return written + snprintf(ptr, rem, ": UNKNOWN_TYPE");
	}
}
