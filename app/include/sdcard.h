#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

bool sdcard_init(void);
bool sdcard_start_session(void);
bool sdcard_end_session(void);
bool sdcard_log_text(const char* text_message);
bool sdcard_open_log(void);
bool sdcard_close_log(void);
bool sdcard_write_str(const char* str);
bool sdcard_log_text(const char* text_message);
bool sdcard_write(const char* buf, uint32_t len);

#ifdef __cplusplus
}
#endif
