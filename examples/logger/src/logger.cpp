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
} buffer_a, buffer_b;

struct logbuffer *write_buf, *read_buf;

static int reader_count = 0;

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

	reader_count = 0;

	write_buf = &buffer_a;
	write_buf->len = 0;

	read_buf = &buffer_b;
	read_buf->len = 0;

	return true;
}


static int append_timestamp(void)
{
	size_t remaining = BUFFER_SIZE - write_buf->len;
	if (remaining == 0) {
		return -1;
	}

	uint64_t now = micros();
	int n;

#ifdef CONFIG_USE_HUMAN_READABLE_TIMESTAMPS
	time_t seconds = (time_t)(now / 1000000ULL);
	uint32_t microseconds = (uint32_t)(now % 1000000ULL);
	struct tm timeinfo;
	localtime_r(&seconds, &timeinfo);
	char time_str[24];
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
	n = snprintf(write_buf->data + write_buf->len, remaining, "[%s.%06" PRIu32 "] ", time_str, microseconds);
#else
	n = snprintf(write_buf->data + write_buf->len, remaining, "[%" PRIu64 "] ", now);
#endif

	if (n < 0 || (size_t)n >= remaining) {
		write_buf->len = BUFFER_SIZE;
		return -1;
	}

	write_buf->len += n;
	return n;
}


static bool logger_swap(void)
{
	if (xSemaphoreTake(read_sem, portMAX_DELAY) != pdTRUE) return false;

	if (reader_count > 0) {
		xSemaphoreGive(read_sem);
		return false;
	}

	struct logbuffer *tmp = write_buf;
	write_buf = read_buf;
	read_buf = tmp;
	write_buf->len = 0;

	xSemaphoreGive(read_sem);
	return true;
}


const char *logger_read_begin(uint32_t *len)
{
	if (xSemaphoreTake(read_sem, portMAX_DELAY) != pdTRUE) return NULL;
	reader_count++;
	if (len) *len = read_buf->len;
	const char *data = read_buf->data;
	xSemaphoreGive(read_sem);
	return data;
}


void logger_read_end(void)
{
	if (xSemaphoreTake(read_sem, portMAX_DELAY) != pdTRUE) return;
	reader_count--;
	xSemaphoreGive(read_sem);
}


int log(const char *fmt, ...)
{
	if (xSemaphoreTake(write_sem, LOG_TIMEOUT) != pdTRUE) return -1;

	size_t remaining = BUFFER_SIZE - write_buf->len;
	if (remaining == 0) {
		// TODO: signal reading tasks that the write buffer is full, forcing them
		//       to flush
		logger_swap();
		remaining = BUFFER_SIZE - write_buf->len;
		if (remaining == 0) {
			xSemaphoreGive(write_sem);
			return -1;
		}
	}

	int ts_len = append_timestamp();
	if (ts_len < 0) {
		logger_swap();
		ts_len = append_timestamp();
		if (ts_len < 0) {
			xSemaphoreGive(write_sem);
			return -1;
		}
	}
	remaining = BUFFER_SIZE - write_buf->len;

	va_list args;
	va_start(args, fmt);
	int n = vsnprintf(write_buf->data + write_buf->len, remaining, fmt, args);
	va_end(args);

	if (n < 0) {
		xSemaphoreGive(write_sem);
		return -1;
	}

	if ((size_t)n >= remaining) {
		write_buf->len = BUFFER_SIZE;
		xSemaphoreGive(write_sem);
		return -1;
	}

	write_buf->len += n;
	xSemaphoreGive(write_sem);
	return n;
}


