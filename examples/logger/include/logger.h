#pragma once

#define LOG_TIMEOUT 10

enum log_destination {
	DEST_UART = 1 << 0,
	DEST_LORA = 1 << 1,
	DEST_SD   = 1 << 2
};

const uint32_t DEST_ALL = DEST_UART | DEST_LORA | DEST_SD;

int log(const char *fmt, ...);
bool logger_init(void);

const char *logger_read_begin(uint32_t *len, int32_t *id);
void logger_read_end(log_destination dest);


#define TO_XSTR(s) TO_STR(s)
#define TO_STR(s) #s

#define LOG(s, ...) log((s) __VA_OPT__(,) __VA_ARGS__)
#define WARN(s, ...) log("[WARN] " s __VA_OPT__(,) __VA_ARGS__)
#define ERR(s, ...) log("[ERR] " s __VA_OPT__(,) __VA_ARGS__)
