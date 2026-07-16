#pragma once

#ifndef MCU_LOG_BUFFER_SIZE
// Size of the buffer for log messages.
#define MCU_LOG_BUFFER_SIZE 4096
#endif

// Log levels
#define MCU_LOG_LEVEL_DEBUG 0
#define MCU_LOG_LEVEL_INFO 1
#define MCU_LOG_LEVEL_WARNING 2
#define MCU_LOG_LEVEL_ERROR 3
#define MCU_LOG_LEVEL_CRITICAL 4
#define MCU_LOG_LEVEL_NONE 5

#if !PRODUCTION
// Default log level
#define MCU_LOG_LEVEL MCU_LOG_LEVEL_DEBUG
#else // PRODUCTION
#define MCU_LOG_LEVEL MCU_LOG_LEVEL_INFO
#endif

#ifndef MCU_LOG_TIMESTAMP_ENABLE
// Enable timestamps in log messages by default
#define MCU_LOG_TIMESTAMP_ENABLE 1
#endif

#ifndef MCU_LOG_TIMESTAMP_ABSOLUTE
// Use wall-clock timestamps (system_clock) instead of relative timestamps
// (time since boot).
#define MCU_LOG_TIMESTAMP_ABSOLUTE 1
#endif

#ifndef MCU_BLE_DEVICE_NAME
// Default Bluetooth device name
#define MCU_BLE_DEVICE_NAME "John StarPi's Rocket"
#endif

#ifndef MCU_TELEMETRY_MAX_CALLBACKS
// Maximum number of callbacks for telemetry manager
#define MCU_TELEMETRY_MAX_CALLBACKS 2
#endif
