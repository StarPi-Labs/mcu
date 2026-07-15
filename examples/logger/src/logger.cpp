#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

#include "logger.h"
#include "task.h"


static LogConsumer consumers[LOG_MAX_CONSUMERS] = {0};
static uint8_t num_consumers = 0;


uint64_t now_us(void)
{
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	return (uint64_t)tv_now.tv_sec * 1000000ULL + tv_now.tv_usec;
}


uint64_t now_ms(void)
{
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	return (uint64_t)tv_now.tv_sec * 1000ULL + tv_now.tv_usec / 1000;
}


// initialize the default message queue, this should be called before using any
// of the other message queue functions
bool logger_init(void)
{
	return true;
}


bool logger_register_consumer(TaskHandle_t task_handle, QueueHandle_t msg_queue, uint32_t payload_filter, uint32_t type_filter)
{
	if (num_consumers >= LOG_MAX_CONSUMERS) return false;

	consumers[num_consumers].task_handle = task_handle;
	consumers[num_consumers].msg_queue = msg_queue;
	consumers[num_consumers].payload_filter = payload_filter;
	consumers[num_consumers].type_filter = type_filter;
	num_consumers++;
	return true;
}


// Sends a message to the correct consumer queues, returns false if any of the
// queues was busy and the message was not written to it
bool logger_sort_message(LogMessage *msg)
{
	bool success = true;

	for (uint8_t i = 0; i < num_consumers; i++) {
		if ((consumers[i].type_filter & msg->type) != 0 && (consumers[i].payload_filter & msg->payload_type) != 0) {
			success &= xQueueSendToFront(consumers[i].msg_queue, msg, LOG_TIMEOUT) == pdPASS;
			xTaskNotifyGive(consumers[i].task_handle);
		}
	}

	return success;
}


size_t logger_message_to_str(const char **str, LogMessage *msg)
{
	static char buf[256];
	int n = 0;

	if (!msg) {
		snprintf(buf, sizeof(buf), "(null)");
		return n;
	}

#ifdef CONFIG_USE_HUMAN_READABLE_TIMESTAMPS
	{
		time_t sec = msg->timestamp / 1000000ULL;
		uint32_t usec = msg->timestamp % 1000000ULL;
		struct tm tm_now;
		char time_str[24];
		localtime_r(&sec, &tm_now);
		strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_now);
		n = snprintf(buf, sizeof(buf), "%s.%06" PRIu32 " ", time_str, usec);
	}
#else
	n = snprintf(buf, sizeof(buf), "%" PRIu64 " ", msg->timestamp);
#endif

	switch (msg->src) {
		case S_IMU:   n += snprintf(buf + n, sizeof(buf) - n, "[IMU] "); break;
		case S_BARO:  n += snprintf(buf + n, sizeof(buf) - n, "[BARO] "); break;
		case S_GPS:   n += snprintf(buf + n, sizeof(buf) - n, "[GPS] "); break;
		case S_LORA:  n += snprintf(buf + n, sizeof(buf) - n, "[LORA] "); break;
		case S_SD:    n += snprintf(buf + n, sizeof(buf) - n, "[SD] "); break;
		case S_OTHER: n += snprintf(buf + n, sizeof(buf) - n, "[OTHER] "); break;
		default:      n += snprintf(buf + n, sizeof(buf) - n, "[UNKNOWN SRC] "); break;
	}

	switch (msg->type) {
		case T_ACCELLERATION: n += snprintf(buf + n, sizeof(buf) - n, "ACCEL: "); break;
		case T_GYRO:          n += snprintf(buf + n, sizeof(buf) - n, "GYRO: "); break;
		case T_ALT_SPEED:     n += snprintf(buf + n, sizeof(buf) - n, "ALT_SPEED: "); break;
		case T_ORIENTATION:   n += snprintf(buf + n, sizeof(buf) - n, "ORIENTATION: "); break;
		case T_PRESSURE:      n += snprintf(buf + n, sizeof(buf) - n, "PRESSURE: "); break;
		case T_TEMPERATURE:   n += snprintf(buf + n, sizeof(buf) - n, "TEMP: "); break;
		case T_GPS:           n += snprintf(buf + n, sizeof(buf) - n, "GPS: "); break;
		case T_SYSLOG:        n += snprintf(buf + n, sizeof(buf) - n, "SYSLOG: "); break;
		default:              n += snprintf(buf + n, sizeof(buf) - n, "UNKNOWN TYPE: "); break;
	}

	switch (msg->payload_type) {
		case P_NONE:
			break;
		case P_BOOL:
			n += snprintf(buf + n, sizeof(buf) - n, "%s", msg->payload.b ? "true" : "false");
			break;
		case P_FLOAT:
			n += snprintf(buf + n, sizeof(buf) - n, "%.3f", (double)msg->payload.f);
			break;
		case P_DOUBLE:
			n += snprintf(buf + n, sizeof(buf) - n, "%.3lf", msg->payload.d);
			break;
		case P_INT:
			n += snprintf(buf + n, sizeof(buf) - n, "%d", msg->payload.i);
			break;
		case P_LONG:
			n += snprintf(buf + n, sizeof(buf) - n, "%ld", msg->payload.l);
			break;
		case P_FVEC2:
			n += snprintf(buf + n, sizeof(buf) - n, "(%.3f, %.3f)", (double)msg->payload.fv2.x, (double)msg->payload.fv2.y);
			break;
		case P_FVEC3:
			n += snprintf(buf + n, sizeof(buf) - n, "(%.3f, %.3f, %.3f)", (double)msg->payload.fv3.x, (double)msg->payload.fv3.y, (double)msg->payload.fv3.z);
			break;
		case P_STRING:
			n += snprintf(buf + n, sizeof(buf) - n, "%s", msg->payload.s ? msg->payload.s : "(null)");
			break;
		default:
			n += snprintf(buf + n, sizeof(buf) - n, "?");
			break;
	}

	n += snprintf(buf+n, sizeof(buf)-n, "\n");

	if (n < 0) {
		buf[0] = '\0';
	} else if (n >= (int)sizeof(buf)) {
		buf[sizeof(buf) - 1] = '\0';
	}

	*str = buf;
	return n;
}


void log(SourceSubsystem src, MessageType type, bool b)
{
	LogMessage msg;
	msg.timestamp = now_us();
	msg.payload_type = P_BOOL;
	msg.src = src;
	msg.type = type;
	msg.payload.b = b;
	logger_sort_message(&msg);
}

void log(SourceSubsystem src, MessageType type, float f)
{
	LogMessage msg;
	msg.timestamp = now_us();
	msg.payload_type = P_FLOAT;
	msg.src = src;
	msg.type = type;
	msg.payload.f = f;
	logger_sort_message(&msg);
}

void log(SourceSubsystem src, MessageType type, double d)
{
	LogMessage msg;
	msg.timestamp = now_us();
	msg.payload_type = P_DOUBLE;
	msg.src = src;
	msg.type = type;
	msg.payload.d = d;
	logger_sort_message(&msg);
}

void log(SourceSubsystem src, MessageType type, int i)
{
	LogMessage msg;
	msg.timestamp = now_us();
	msg.payload_type = P_INT;
	msg.src = src;
	msg.type = type;
	msg.payload.i = i;
	logger_sort_message(&msg);
}

void log(SourceSubsystem src, MessageType type, long l)
{
	LogMessage msg;
	msg.timestamp = now_us();
	msg.payload_type = P_LONG;
	msg.src = src;
	msg.type = type;
	msg.payload.l = l;
	logger_sort_message(&msg);
}

void log(SourceSubsystem src, MessageType type, float x, float y)
{
	LogMessage msg;
	msg.timestamp = now_us();
	msg.payload_type = P_FVEC2;
	msg.src = src;
	msg.type = type;
	msg.payload.fv2.x = x;
	msg.payload.fv2.y = y;
	logger_sort_message(&msg);
}

void log(SourceSubsystem src, MessageType type, float x, float y, float z)
{
	LogMessage msg;
	msg.timestamp = now_us();
	msg.payload_type = P_FVEC3;
	msg.src = src;
	msg.type = type;
	msg.payload.fv3.x = x;
	msg.payload.fv3.y = y;
	msg.payload.fv3.z = z;
	logger_sort_message(&msg);
}

void log(SourceSubsystem src, MessageType type, const char *s)
{
	LogMessage msg;
	msg.timestamp = now_us();
	msg.payload_type = P_STRING;
	msg.src = src;
	msg.type = type;
	msg.payload.s = s;
	logger_sort_message(&msg);
}
