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
	MSG_IVEC3,
} message_type_t;

struct vec3 {
	float x, y, z;
};

struct ivec3 {
	int32_t x, y, z;
};

enum message_dest_t : uint8_t {
	DEST_NONE = 0,
	DEST_UART = 1 << 0,
	DEST_SD   = 1 << 1,
	DEST_LORA = 1 << 2,
	DEST_ALL  = 0xFF,
};

// TODO: align or pack this struct
typedef struct {
	uint64_t timestamp; // unix timestamp in microseconds
	message_type_t type;
	uint8_t dest;
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
		struct ivec3 iv3;
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


inline message_t message_create(const char* str, uint8_t dest) {
	message_t msg = {};
	msg.timestamp = get_timestamp();
	msg.type = MSG_NONE;
	msg.dest = dest;
	msg.description = str;
	return msg;
}

inline message_t message_create(const char* str, uint8_t dest, int32_t data) {
	message_t msg = message_create(str, dest);
	msg.type = MSG_INT32;
	msg.data.i32 = data;
	return msg;
}

inline message_t message_create(const char* str, uint8_t dest, uint32_t data) {
	message_t msg = message_create(str, dest);
	msg.type = MSG_UINT32;
	msg.data.u32 = data;
	return msg;
}

inline message_t message_create(const char* str, uint8_t dest, float data) {
	message_t msg = message_create(str, dest);
	msg.type = MSG_FLOAT;
	msg.data.f = data;
	return msg;
}

inline message_t message_create(const char* str, uint8_t dest, double data) {
	message_t msg = message_create(str, dest);
	msg.type = MSG_DOUBLE;
	msg.data.d = data;
	return msg;
}

inline message_t message_create(const char* str, uint8_t dest, const char* data) {
	message_t msg = message_create(str, dest);
	msg.type = MSG_STRING;
	msg.data.str = data;
	return msg;
}

inline message_t message_create(const char* str, uint8_t dest, struct vec3 data) {
	message_t msg = message_create(str, dest);
	msg.type = MSG_VEC3;
	msg.data.v3 = data;
	return msg;
}

inline message_t message_create(const char* str, uint8_t dest, struct ivec3 data) {
	message_t msg = message_create(str, dest);
	msg.type = MSG_IVEC3;
	msg.data.iv3 = data;
	return msg;
}

// Number of elements in a queue
#define MESSAGE_QUEUE_SIZE 128


// Function declarations
bool message_queue_init();
bool message_queue_reset(message_dest_t dest);
bool message_queue_enqueue(message_t *message, TickType_t timeout);
bool message_queue_dequeue(message_t *message, TickType_t timeout, message_dest_t dest);
bool message_queue_peek(message_t *message, TickType_t timeout, message_dest_t dest);
bool message_queue_full(message_dest_t dest);
int message_queue_items(message_dest_t dest);
int format_message_to_string(const message_t *msg, char *buf, size_t size);


// ugly macros
#define WARN_STR(s) "[WARNING]: " s
#define ERR_STR(s)  "[ERROR]: " s
#define LOG_STR(s)  s


#define TO_XSTR(s) TO_STR(s)
#define TO_STR(s) #s
#define ERR_TIMEOUT 0
#define ERR(dest, str, ...) do { \
	message_t msg = message_create(ERR_STR(str), (dest) __VA_OPT__(,) __VA_ARGS__); \
	message_queue_enqueue(&msg, ERR_TIMEOUT); \
} while(0)

#define WARN(dest, str, ...) do { \
	message_t msg = message_create(WARN_STR(str), (dest) __VA_OPT__(,) __VA_ARGS__); \
	message_queue_enqueue(&msg, ERR_TIMEOUT); \
} while(0)

#define LOG(dest, str, ...) do { \
	message_t msg = message_create(LOG_STR(str), (dest) __VA_OPT__(,) __VA_ARGS__); \
	message_queue_enqueue(&msg, ERR_TIMEOUT); \
} while(0)
