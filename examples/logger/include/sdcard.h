#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool sdcard_init(void);
bool sdcard_start_session(void);
bool sdcard_end_session(void);
bool sdcard_log_text(const char* text_message);

#ifdef __cplusplus
}
#endif
