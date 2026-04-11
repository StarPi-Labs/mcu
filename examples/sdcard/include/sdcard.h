#pragma once

#include <stddef.h>
#include "logging.h"

#ifdef __cplusplus
extern "C" {
#endif

bool sdcard_init(void);
bool sdcard_start_session(void);
bool sdcard_end_session(void);
bool sdcard_log(const void* message, const MessageDescriptor* desc);

#ifdef __cplusplus
}
#endif
