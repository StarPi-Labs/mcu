#pragma once

#define LOG_TIMEOUT 10

int log(const char *fmt, ...);
bool logger_init(void);

const char *logger_read_begin(uint32_t *len);
void logger_read_end(void);


#define TO_XSTR(s) TO_STR(s)
#define TO_STR(s) #s

#define LOG(s, ...) log((s) __VA_OPT__(,) __VA_ARGS__)
#define WARN(s, ...) log("[WARN] ", (s) __VA_OPT__(,) __VA_ARGS__)
#define ERR(s, ...) log("[ERR] ", (s) __VA_OPT__(,) __VA_ARGS__)
