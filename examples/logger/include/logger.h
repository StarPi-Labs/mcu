#pragma once

#include <FreeRTOS.h>


typedef enum {
	MSG_NONE = 0, // No data, just a timestamp and eventual message
	MSG_INT32,
	MSG_UINT32,
	MSG_INT64,
	MSG_UINT64,
	MSG_FLOAT,
	MSG_DOUBLE,
	MSG_STRING,
	MSG_VEC3,
} message_type_t;


struct vec3 {
	float x, y, z;
};


// TODO: define a timestamp standard
// TODO: align or pack this struct
typedef struct {
	unsigned long timestamp;
	message_type_t type;
	const char *description;
	union {
		int32_t i32;
		uint32_t u32;
		int64_t i64;
		uint64_t u64;
		float f;
		double d;
		const char *str;
		struct vec3 v3;
	} data;
} message_t;


// TODO: for now this is a simple wrapper around FreeRTOS queues
// https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues
typedef QueueHandle_t message_queue_t;

// Number of elements in a queue
#define MESSAGE_QUEUE_SIZE 128

// The symbol of the message queue
extern message_queue_t DEFAULT_QUEUE;

// Function declarations
bool message_queue_init();
bool message_queue_reset();
bool message_queue_enqueue(message_t *message, TickType_t timeout);
bool message_queue_dequeue(message_t *message, TickType_t timeout);
bool message_queue_peek(message_t *message, TickType_t timeout);


// ugly macros
#define WARN_STR(s) "[WARNING]: " ## s
#define ERR_STR(s)  "[ERROR]: " ## s
#define LOG_STR(s)  s
