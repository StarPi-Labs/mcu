#pragma once

#include <Arduino.h>
#include <ConditionVariable.h>
#include <Mutex.h>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <format>
#include <functional>
#include <string_view>
#include <vector>

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
// Use absolute timestamps (time since boot) instead of relative timestamps
#define MCU_LOG_TIMESTAMP_ABSOLUTE 1
#endif

#define STRINGIFY(x) _STRINGIFY(x)
#define _STRINGIFY(x) #x

namespace mcu::log
{
namespace implementation
{
inline TaskHandle_t g_loggerTaskHandle; ///< Handle for the logger task, used
                                        ///< for task notifications.
inline constexpr UBaseType_t BUFFER_FULL_BIT =
    0x80000000; //< Bit mask to indicate buffer full state in task
                // notifications.

#if MCU_LOG_TIMESTAMP_ENABLE && !MCU_LOG_TIMESTAMP_ABSOLUTE
inline std::chrono::time_point<std::chrono::steady_clock>
    g_bootTime; ///< Time point representing system boot
                ///< time.
#endif

using Buffer = std::array<char, MCU_LOG_BUFFER_SIZE>;
inline std::array<Buffer, 2> g_buffers;
inline std::uint8_t g_activeIndex = 0;
inline std::size_t g_bufferOffset = 0;
inline std::size_t g_inactiveBufferOffset = 0;
inline freertos::Mutex g_mutex;
inline freertos::Mutex g_inactiveMutex;
inline freertos::ConditionVariable g_cvBufferFull, g_cvBufferEmpty;
} // namespace implementation

// ----- Public API ------
//  ----- Prefixes -------
constexpr std::string_view PREFIX_DEBUG = "[DEBUG]: ";
static_assert(PREFIX_DEBUG.size() < MCU_LOG_BUFFER_SIZE);

constexpr std::string_view PREFIX_INFO = "[INFO]: ";
static_assert(PREFIX_INFO.size() < MCU_LOG_BUFFER_SIZE);

constexpr std::string_view PREFIX_WARNING = "[WARNING]: ";
static_assert(PREFIX_WARNING.size() < MCU_LOG_BUFFER_SIZE);

constexpr std::string_view PREFIX_ERROR = "[ERROR]: ";
static_assert(PREFIX_ERROR.size() < MCU_LOG_BUFFER_SIZE);

constexpr std::string_view PREFIX_CRITICAL = "[CRITICAL]: ";
static_assert(PREFIX_CRITICAL.size() < MCU_LOG_BUFFER_SIZE);

// ----- Implementation ------
using Handler =
    std::function<void(std::string_view)>; ///< Type for log handler functions.

/// @brief Structure representing a log target, which includes a log handler
/// function and its associated FreeRTOS task priority.
struct Target {
  const char* name;
  Handler handler;
  UBaseType_t priority;
  std::uint32_t stackSize;
};

namespace implementation
{
inline std::vector<Target>
    g_targets; ///< List of log targets to which formatted messages are sent.
} // namespace implementation

/// @brief Initializes the logging system, including starting the logger task.
///
/// @pre @c Serial must be initialized.
void init();

/// @brief Logs (prints) a formatted message with a prefix and adds a newline at
/// the end.
///
/// It is not intended to be used directly, but via macros:
/// mcu_log_debug, mcu_log_info, mcu_log_warning, mcu_log_error,
/// mcu_log_critical.
/// @param logLevel The log level string (e.g. "[DEBUG]").
/// @param format The format string (uses std::format_string,
/// for info on syntax @see
/// https://en.cppreference.com/w/cpp/utility/format/spec.html).
/// @param args Variable arguments pack to format into the string.
template <typename... Args>
inline void
logf(const std::string_view& logLevel,
     const std::format_string<std::type_identity_t<Args>...>& format,
     Args&&... args)
{
  using namespace implementation;

  {
    std::unique_lock lock(g_mutex);

#if MCU_LOG_TIMESTAMP_ENABLE
    auto timestamp =
#if MCU_LOG_TIMESTAMP_ABSOLUTE
        std::chrono::system_clock::now()
#else
        std::chrono::steady_clock::now() - g_bootTime
#endif
        ;

    // e.g. ABSOLUTE: "2023-10-01T15:42:42.1234+0200", RELATIVE: "15:42:42.1234"
    constexpr std::format_string<decltype(timestamp)&> TIMESTAMP_FORMAT =
#if MCU_LOG_TIMESTAMP_ABSOLUTE
        "{0:%F}T{0:%T%z} "
#else
        "{:%T} "
#endif
        ;

    std::size_t timestampSize =
        std::formatted_size(TIMESTAMP_FORMAT, timestamp);
#endif

    std::size_t formatSize =
        std::formatted_size(format, std::forward<Args>(args)...);

    std::size_t requiredSize = logLevel.size() + formatSize +
                               1 // +1 for newline
#if MCU_LOG_TIMESTAMP_ENABLE
                               + timestampSize
#endif
        ;

    assert(requiredSize <= MCU_LOG_BUFFER_SIZE &&
           "Log message is too large to fit in the buffer");

    g_cvBufferFull.wait(lock, [&] {
      if (!(g_bufferOffset + requiredSize <= MCU_LOG_BUFFER_SIZE)) {
        // Buffer is full, notify logger task to flush it
        xTaskNotify(g_loggerTaskHandle, BUFFER_FULL_BIT, eSetBits);
        return false; // Wait until buffer is flushed
      }

      return true; // We have enough space to write the message
    });

    Buffer& activeBuffer = g_buffers[g_activeIndex];

    // Timestamp
#if MCU_LOG_TIMESTAMP_ENABLE
    std::format_to(&activeBuffer[g_bufferOffset], TIMESTAMP_FORMAT, timestamp);
    g_bufferOffset += timestampSize;
#endif

    // Log level prefix
    std::memcpy(&activeBuffer[g_bufferOffset], logLevel.data(),
                logLevel.size());
    g_bufferOffset += logLevel.size();

    // Message
    std::format_to(&activeBuffer[g_bufferOffset], format,
                   std::forward<Args>(args)...);
    g_bufferOffset += formatSize;

    // Newline
    std::memcpy(&activeBuffer[g_bufferOffset], "\n", 1);
    g_bufferOffset += 1;
  }

  g_cvBufferEmpty.notify_one();
}

/// @brief FreeRTOS task to manage printing log messages.
///
/// This task takes formatted messages from `Buffer`
/// and sends them to all registered handlers in `Handlers`.
/// @param pvParams Task parameters (not used in this case).
void vTask(void* pvParams);

/// @brief Forces immediate printing of all log messages currently in the
/// buffer.
void flush();

/// @brief Adds a new log target to receive log messages atomically.
/// Should be called before init()
/// @param name Name of the log target (for debugging purposes).
/// @param handler The log handler function to call with formatted messages.
/// @param priority The FreeRTOS task priority for the handler (if it needs to
/// be run in a separate task).
/// @param stackSize The stack size for the handler task, if applicable.
void addTarget(const char* name, Handler handler, UBaseType_t priority,
               std::uint32_t stackSize);
} // namespace mcu::log

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_DEBUG
/// @brief Logs a message at DEBUG level.
/// Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_DEBUG.
/// @param format Format string compatible with std::format.
/// @param ... Any arguments to format.
#define mcu_log_debug(format, ...)                                             \
  ::mcu::log::logf(::mcu::log::PREFIX_DEBUG, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_debug(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_INFO
/// @brief Logs a message at INFO level.
/// Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_INFO.
/// @param format Format string compatible with std::format.
/// @param ... Any arguments to format.
#define mcu_log_info(format, ...)                                              \
  ::mcu::log::logf(::mcu::log::PREFIX_INFO, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_info(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_WARNING
/// @brief Logs a message at WARNING level.
/// Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_WARNING.
/// @param format Format string compatible with std::format.
/// @param ... Optional arguments to format.
#define mcu_log_warning(format, ...)                                           \
  ::mcu::log::logf(::mcu::log::PREFIX_WARNING,                                 \
                   format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_warning(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_ERROR
/// @brief Logs a message at ERROR level.
/// Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_ERROR.
/// @param format Format string compatible with std::format.
/// @param ... Optional arguments to format.
#define mcu_log_error(format, ...)                                             \
  ::mcu::log::logf(::mcu::log::PREFIX_ERROR, format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_error(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_CRITICAL
/// @brief Logs a message at CRITICAL level.
/// Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_CRITICAL.
/// @param format Format string compatible with std::format.
/// @param ... Optional arguments to format.
#define mcu_log_critical(format, ...)                                          \
  ::mcu::log::logf(::mcu::log::PREFIX_CRITICAL,                                \
                   format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_critical(format, ...)
#endif
