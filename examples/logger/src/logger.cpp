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

	write_buf = &buffer_a;
	write_buf->len = 0;

	read_buf = &buffer_b;
	read_buf->len = 0;

	return true;
}


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

	xSemaphoreGive(read_sem);
	return true;
}


const char *logger_read_begin(uint32_t *len, int32_t *id)
{
	if (xSemaphoreTake(read_sem, portMAX_DELAY) != pdTRUE) return NULL;
	if (len) *len = read_buf->len;
	if (id) *id = read_buf->id;
	const char *data = read_buf->data;
	xSemaphoreGive(read_sem);
	return data;
}


void logger_read_end(log_destination dest)
{
	if (xSemaphoreTake(read_sem, portMAX_DELAY) != pdTRUE) return;
	reader_pending &= ~dest;

	if (reader_pending == 0) {
		pending_swap = true;
	}

	xSemaphoreGive(read_sem);
}


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


