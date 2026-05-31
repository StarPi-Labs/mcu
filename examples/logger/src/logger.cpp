#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

#include "logger.h"
#include "task.h"


#define BUFFER_SIZE 2048
struct logbuffer {
	char data[BUFFER_SIZE];
	uint32_t len;
	int32_t id;
};

struct logbuffer buffer_a, buffer_b;
struct logbuffer *write_buf, *read_buf;

static int reader_pending;
static volatile bool pending_swap = false;

static size_t       readers_num = 0;
static TaskHandle_t readers[LOG_MAX_READERS] = {};

DECLARE_STATIC_SEMAPHORE(write_sem);
DECLARE_STATIC_SEMAPHORE(read_sem);


// initialize the default message queue, this should be called before using any
// of the other message queue functions
bool logger_init(void)
{
	INIT_STATIC_SEMAPHORE(write_sem);
	INIT_STATIC_SEMAPHORE(read_sem);
	if (write_sem == NULL || read_sem == NULL) {
		return false;
	}
	xSemaphoreGive(write_sem);
	xSemaphoreGive(read_sem);

	reader_pending = DEST_ALL;
	pending_swap = false;
	readers_num = 0;

	write_buf = &buffer_a;
	write_buf->len = 0;

	read_buf = &buffer_b;
	read_buf->len = 0;

	return true;
}


// Register a reader task, this task will get notified when a swap happens, which
// indicates that new data is available to be read
void logger_register(TaskHandle_t handle)
{
	if (handle != NULL && readers_num < LOG_MAX_READERS) {
		readers[readers_num++] = handle;
	}
}


// Swap the current write buffer with the read buffer, only happens when all
// pending readers are done. Notifies all registered reader tasks with a FreeRTOS
// notification.
// This resets the pendign destinations to DEST_ALL
static bool logger_swap(void)
{
	if (xSemaphoreTake(read_sem, portMAX_DELAY) != pdTRUE) return false;

	if (reader_pending != 0) {
		xSemaphoreGive(read_sem);
		return false;
	}

	struct logbuffer *tmp = write_buf;
	write_buf = read_buf;
	read_buf = tmp;
	write_buf->len = 0;
	write_buf->id++;
	reader_pending = DEST_ALL;

	// Notify all readers
	for (size_t i = 0; i < readers_num; i++) {
		xTaskNotifyGive(readers[i]);
	}

	xSemaphoreGive(read_sem);
	return true;
}


// Called by a reader returns the current read buffer, it's length and it's id
// The id can be used by tasks which need to run periodically to check wether the
// buffer they got is the same as the previous one
const char *logger_read_begin(uint32_t *len, int32_t *id)
{
	if (xSemaphoreTake(read_sem, portMAX_DELAY) != pdTRUE) return NULL;
	if (len) *len = read_buf->len;
	if (id) *id = read_buf->id;
	const char *data = read_buf->data;
	xSemaphoreGive(read_sem);
	return data;
}


// Called by a reader, signals that it is done with the read buffer, removes it's
// destination from pending
void logger_read_end(log_destination dest)
{
	if (xSemaphoreTake(read_sem, portMAX_DELAY) != pdTRUE) return;
	reader_pending &= ~dest;

	if (reader_pending == 0) {
		pending_swap = true;
	}

	xSemaphoreGive(read_sem);
}


// Appends a timestamp in microseconds to the buffer, returns how many bytes were
// written to the buffer.
// The timestamp is either an int or a human readable date depending on the
// compile-time options
static int append_timestamp(char *buf, uint32_t len)
{
	uint64_t now = micros();
	int n;

#ifdef CONFIG_USE_HUMAN_READABLE_TIMESTAMPS
	time_t seconds = (time_t)(now / 1000000ULL);
	uint32_t microseconds = (uint32_t)(now % 1000000ULL);
	struct tm timeinfo;
	localtime_r(&seconds, &timeinfo);
	char time_str[24];
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
	n = snprintf(buf, len, "%s.%06" PRIu32 " ", time_str, microseconds);
#else
	n = snprintf(buf, len, "%" PRIu64 " ", now);
#endif

	return n;
}


// Takes the same arguments as printf, pushes the formatted string to the write
// buffer and automatically appends a timestamp
int log(const char *fmt, ...)
{

	static char temp_buf[256];
	static uint32_t temp_len;

	if (xSemaphoreTake(write_sem, LOG_TIMEOUT) != pdTRUE) return -1;

	if (pending_swap) {
		pending_swap = false;
		if (write_buf->len > 0) {
			logger_swap();
		}
	}

	temp_len = 0;
	temp_len += append_timestamp(temp_buf, 256);
	va_list args;
	va_start(args, fmt);
	temp_len += vsnprintf(temp_buf + temp_len, 256-temp_len, fmt, args);
	if (temp_len < 255) temp_buf[temp_len++] = '\n';
	va_end(args);

	if (temp_len > 256) {
		xSemaphoreGive(write_sem);
		return -1;
	}

	uint32_t remaining = BUFFER_SIZE - write_buf->len;
	if (temp_len > remaining) {
		// TODO: signal reading tasks that the write buffer is full, forcing them
		//       to flush
		if (!logger_swap()) {
			xSemaphoreGive(write_sem);
			return -1;
		}
	}

	memcpy(write_buf->data + write_buf->len, temp_buf, temp_len);

	write_buf->len += temp_len;
	xSemaphoreGive(write_sem);
	return temp_len;
}


