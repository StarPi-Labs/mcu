#include <Arduino.h>

#include "logger.h"

// TODO: this could be in PSRAM if we had it
static uint8_t default_queue_buffer[MESSAGE_QUEUE_SIZE * sizeof(message_t)];
static StaticQueue_t default_queue_desc;

message_queue_t DEFAULT_QUEUE = NULL;


bool message_queue_init()
{
	DEFAULT_QUEUE = xQueueCreateStatic(MESSAGE_QUEUE_SIZE, sizeof(message_t), default_queue_buffer, &default_queue_desc);
	if (DEFAULT_QUEUE == NULL) return false;
	return true;
}


bool message_queue_reset()
{
	if (DEFAULT_QUEUE == NULL) return false;
	xQueueReset(DEFAULT_QUEUE);
	return true;
}


// push the message to the back of the queue, this should not be called from an ISR
bool message_queue_enqueue(message_t *message, TickType_t timeout)
{
	if (DEFAULT_QUEUE == NULL || message == NULL) return false;
	if (xQueueSend(DEFAULT_QUEUE, message, timeout) != pdPASS) return false;
	return true;
}


bool message_queue_dequeue(message_t *message, TickType_t timeout)
{
	if (DEFAULT_QUEUE == NULL || message == NULL) return false;
	if (xQueueReceive(DEFAULT_QUEUE, message, timeout) != pdPASS) return false;
	return true;
}


bool message_queue_peek(message_t *message, TickType_t timeout)
{
	if (DEFAULT_QUEUE == NULL || message == NULL) return false;
	if (xQueuePeek(DEFAULT_QUEUE, message, timeout) != pdPASS) return false;
	return true;
}
