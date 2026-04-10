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

// TODO: align or pack this struct
typedef struct {
	uint64_t timestamp; // unix timestamp in microseconds
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

// get the current timestamp in microseconds since the epoch, this is used for
// the message timestamp
static inline uint64_t get_timestamp(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec;
}


#ifdef __cplusplus

// Overloads MUST be outside extern "C" because C doesn't support overloading
inline message_t create_msg_impl(const char* str) {
	message_t msg = {};
	msg.timestamp = get_timestamp();
	msg.type = MSG_NONE;
	msg.description = str;
	return msg;
}

inline message_t create_msg_impl(const char* str, int32_t data) {
	message_t msg = create_msg_impl(str);
	msg.type = MSG_INT32;
	msg.data.i32 = data;
	return msg;
}

inline message_t create_msg_impl(const char* str, uint32_t data) {
	message_t msg = create_msg_impl(str);
	msg.type = MSG_UINT32;
	msg.data.u32 = data;
	return msg;
}

inline message_t create_msg_impl(const char* str, float data) {
	message_t msg = create_msg_impl(str);
	msg.type = MSG_FLOAT;
	msg.data.f = data;
	return msg;
}

inline message_t create_msg_impl(const char* str, double data) {
	message_t msg = create_msg_impl(str);
	msg.type = MSG_DOUBLE;
	msg.data.d = data;
	return msg;
}

inline message_t create_msg_impl(const char* str, const char* data) {
	message_t msg = create_msg_impl(str);
	msg.type = MSG_STRING;
	msg.data.str = data;
	return msg;
}

inline message_t create_msg_impl(const char* str, struct vec3 data) {
	message_t msg = create_msg_impl(str);
	msg.type = MSG_VEC3;
	msg.data.v3 = data;
	return msg;
}

// The unified C++ macro
#define MESSAGE(...) create_msg_impl(__VA_ARGS__)

// ============================================================================
// 3. C11 IMPLEMENTATION (Using _Generic)
// ============================================================================
#else

#define MESSAGE_WITH_DATA(str, val) _Generic((val), \
	int32_t:     (message_t){.timestamp = get_timestamp(), .type = MSG_INT32,  .description = (str), .data.i32 = (val)}, \
	uint32_t:    (message_t){.timestamp = get_timestamp(), .type = MSG_UINT32, .description = (str), .data.u32 = (val)}, \
	int64_t:     (message_t){.timestamp = get_timestamp(), .type = MSG_INT64,  .description = (str), .data.i64 = (val)}, \
	uint64_t:    (message_t){.timestamp = get_timestamp(), .type = MSG_UINT64, .description = (str), .data.u64 = (val)}, \
	float:       (message_t){.timestamp = get_timestamp(), .type = MSG_FLOAT,  .description = (str), .data.f   = (val)}, \
	double:      (message_t){.timestamp = get_timestamp(), .type = MSG_DOUBLE, .description = (str), .data.d   = (val)}, \
	char*:       (message_t){.timestamp = get_timestamp(), .type = MSG_STRING, .description = (str), .data.str = (val)}, \
	const char*: (message_t){.timestamp = get_timestamp(), .type = MSG_STRING, .description = (str), .data.str = (val)}, \
	struct vec3: (message_t){.timestamp = get_timestamp(), .type = MSG_VEC3,   .description = (str), .data.v3  = (val)} \
)

#define MESSAGE_NO_DATA(str) (message_t){.timestamp = get_timestamp(), .type = MSG_NONE, .description = (str)}

#define MESSAGE_CHOOSER(_1, _2, NAME, ...) NAME

// The unified C macro
#define MESSAGE(...) MESSAGE_CHOOSER(__VA_ARGS__, MESSAGE_WITH_DATA, MESSAGE_NO_DATA, dummy)(__VA_ARGS__)

#endif // __cplusplus

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
int format_message_to_string(const message_t *msg, char *buf, size_t size);


// ugly macros
#define WARN_STR(s) "[WARNING]: " ## s
#define ERR_STR(s)  "[ERROR]: " ## s
#define LOG_STR(s)  s
