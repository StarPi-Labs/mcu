#pragma once

#include <Arduino.h>
#include <ConditionVariable.h>
#include <Mutex.h>
#include <array>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <format>
#include <functional>
#include <string_view>

#ifndef MCU_LOG_BUFFER_SIZE
// Size of the buffer for log messages.
// NOTE: It must be at least as large as the longest prefix.
#define MCU_LOG_BUFFER_SIZE 4096
#endif

// Log levels
#define MCU_LOG_LEVEL_DEBUG 0
#define MCU_LOG_LEVEL_INFO 1
#define MCU_LOG_LEVEL_WARNING 2
#define MCU_LOG_LEVEL_ERROR 3
#define MCU_LOG_LEVEL_CRITICAL 4
#define MCU_LOG_LEVEL_NONE 5

#if DEBUG || RELEASE
// Default log level
#define MCU_LOG_LEVEL MCU_LOG_LEVEL_DEBUG
#else // PRODUCTION
#define MCU_LOG_LEVEL MCU_LOG_LEVEL_INFO
#endif

namespace mcu
{
// Prefixes
constexpr std::string_view LOG_PREFIX_DEBUG = "[DEBUG]: ";
constexpr std::string_view LOG_PREFIX_INFO = "[INFO]: ";
constexpr std::string_view LOG_PREFIX_WARNING = "[WARNING]: ";
constexpr std::string_view LOG_PREFIX_ERROR = "[ERROR]: ";
constexpr std::string_view LOG_PREFIX_CRITICAL = "[CRITICAL]: ";

namespace LogTargets
{
using LogHandler = std::function<void(std::string_view)>;

struct LogTarget {
  LogHandler handler;
  UBaseType_t priority;
};

const LogTarget ARDUINO_SERIAL = {
    [](std::string_view msg) {
      Serial.write(reinterpret_cast<const uint8_t*>(msg.data()), msg.size());
    },
    tskIDLE_PRIORITY + 1};
} // namespace LogTargets

// Log targets to which the message is sent after
// formatting, note it is not intended to be modified at runtime
// but only during program initialization (e.g. setup())
inline std::vector<LogTargets::LogTarget> g_logTargets;

namespace implementation
{
using LogBuffer_t = std::array<char, MCU_LOG_BUFFER_SIZE>;
inline std::array<LogBuffer_t, 2> g_logBuffers;
inline std::uint8_t g_logActiveIndex = 0;
inline std::size_t g_bufferOffset = 0;
inline freertos::Mutex g_logMutex;
inline freertos::ConditionVariable g_cvLogFull, g_cvLogEmpty;
} // namespace implementation

/**
 * @brief Logs (prints) a formatted message with a prefix.
 *
 * It is not intended to be used directly, but via macros:
 * mcu_log_debug, mcu_log_info, mcu_log_warning, mcu_log_error,
 mcu_log_critical.

 * @param logLevel The log level string (e.g. "[DEBUG]").
 * @param format The format string (uses std::format_string,
 * for info on syntax @see
 https://en.cppreference.com/w/cpp/utility/format/spec.html).
 * @param args... Variable arguments to format into the string.
 */
template <typename... Args>
inline void
logf(const std::string_view& logLevel,
     const std::format_string<std::type_identity_t<Args>...>& format,
     Args&&... args)
{
  using namespace implementation;

  {
    std::unique_lock lock(g_logMutex);

    std::size_t formattedSize =
        std::formatted_size(format, std::forward<Args>(args)...);

    std::size_t requiredSize = logLevel.size() + formattedSize;

    assert(requiredSize <= MCU_LOG_BUFFER_SIZE &&
           "Log message is too large to fit in the buffer");

    g_cvLogFull.wait(lock, [&] {
      return g_bufferOffset + requiredSize <= MCU_LOG_BUFFER_SIZE;
    });

    LogBuffer_t& activeBuffer = g_logBuffers[g_logActiveIndex];

    std::memcpy(&activeBuffer[g_bufferOffset], logLevel.data(),
                logLevel.size());

    g_bufferOffset += logLevel.size();

    // Format the message directly into the buffer
    std::format_to(&activeBuffer[g_bufferOffset], format,
                   std::forward<Args>(args)...);

    g_bufferOffset += formattedSize;
  }

  g_cvLogEmpty.notify_one();
}

/**
 * @brief FreeRTOS task to manage printing log messages.
 *
 * This task takes formatted messages from `g_logBuffer`
 * and sends them to all registered handlers in `g_logHandlers`.
 *
 * @param pvParams Task parameters (not used in this case).
 */
void vTaskLogger(void* pvParams);

/**
 * @brief Forces immediate printing of all log messages currently in the buffer.
 */
void flush();

} // namespace mcu

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_DEBUG
/**
 * @brief Logs a message at DEBUG level.
 * If enabled, adds the [DEBUG] prefix to the head of the log.
 * Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_DEBUG.
 *
 * @param format Format string compatible with std::format.
 * @param ... Any arguments to format.
 */
#define mcu_log_debug(format, ...)                                             \
  ::mcu::logf(::mcu::LOG_PREFIX_DEBUG, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_debug(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_INFO
/**
 * @brief Logs a message at INFO level.
 * If enabled, adds the [INFO] prefix to the head of the log.
 * Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_INFO.
 *
 * @param format Format string compatible with std::format.
 * @param ... Any arguments to format.
 */
#define mcu_log_info(format, ...)                                              \
  ::mcu::logf(::mcu::LOG_PREFIX_INFO, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_info(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_WARNING
/**
 * @brief Logs a message at WARNING level.
 * If enabled, adds the [WARNING] prefix to the head of the log.
 * Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_WARNING.
 *
 * @param format Format string compatible with std::format.
 * @param ... Optional arguments to format.
 */
#define mcu_log_warning(format, ...)                                           \
  ::mcu::logf(::mcu::LOG_PREFIX_WARNING, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_warning(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_ERROR
/**
 * @brief Logs a message at ERROR level.
 * If enabled, adds the [ERROR] prefix to the head of the log.
 * Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_ERROR.
 *
 * @param format Format string compatible with std::format.
 * @param ... Optional arguments to format.
 */
#define mcu_log_error(format, ...)                                             \
  ::mcu::logf(::mcu::LOG_PREFIX_ERROR, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_error(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_CRITICAL
/**
 * @brief Logs a message at CRITICAL level.
 * If enabled, adds the [CRITICAL] prefix to the head of the log.
 * Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_CRITICAL.
 *
 * @param format Format string compatible with std::format.
 * @param ... Optional arguments to format.
 */
#define mcu_log_critical(format, ...)                                          \
  ::mcu::logf(::mcu::LOG_PREFIX_CRITICAL, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_critical(format, ...)
#endif