#pragma once

#include <Arduino.h>
#include <ConditionVariable.h>
#include <Mutex.h>
#include <array>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <format>
#include <functional>
#include <string_view>
#include <type_traits>
#include <vector>

#include "CoreConfig.h"
#include "Telemetry.h"

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
constexpr std::string_view DEBUG_STRING = "debug";
static_assert(DEBUG_STRING.size() < MCU_LOG_BUFFER_SIZE);

constexpr std::string_view INFO_STRING = "info";
static_assert(INFO_STRING.size() < MCU_LOG_BUFFER_SIZE);

constexpr std::string_view WARNING_STRING = "warning";
static_assert(WARNING_STRING.size() < MCU_LOG_BUFFER_SIZE);

constexpr std::string_view ERROR_STRING = "error";
static_assert(ERROR_STRING.size() < MCU_LOG_BUFFER_SIZE);

constexpr std::string_view CRITICAL_STRING = "critical";
static_assert(CRITICAL_STRING.size() < MCU_LOG_BUFFER_SIZE);

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
/// the end:
// <timestamp> <facility>.<log level> <task name>[<task number>]: <format>
///
/// It is not intended to be used directly, but via macros:
/// mcu_log_debug, mcu_log_info, mcu_log_warning, mcu_log_error,
/// mcu_log_critical.
/// @param facility Facility string (e.g. "main").
/// @param logLevel The log level string (e.g. "[DEBUG]").
/// @param format The format string (uses std::format_string,
/// for info on syntax @see
/// https://en.cppreference.com/w/cpp/utility/format/spec.html).
/// @param args Variable arguments pack to format into the string.
template <typename... Args>
inline void
logf(std::string_view facility, std::string_view logLevel,
     const std::format_string<std::type_identity_t<Args>...>& format,
     Args&&... args)
{
  assert(!facility.empty() && "Facility string must not be empty");
  assert(!logLevel.empty() && "Log level string must not be empty");

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
        "{0:%F}T{0:%T%z}"
#else
        "{:%T}"
#endif
        ;

    std::size_t timestampSize =
        std::formatted_size(TIMESTAMP_FORMAT, timestamp);
#endif

    // Get task info
    std::string_view taskName(pcTaskGetName(NULL));
    UBaseType_t taskNumber = uxTaskGetTaskNumber(NULL);
    constexpr std::format_string<decltype(taskName)&, decltype(taskNumber)&>
        TASK_INFO_FORMAT = "{}[{}]";

    std::size_t taskInfoSize =
        std::formatted_size(TASK_INFO_FORMAT, taskName, taskNumber);

    std::size_t formatSize =
        std::formatted_size(format, std::forward<Args>(args)...);

    std::size_t requiredSize =
#if MCU_LOG_TIMESTAMP_ENABLE
        timestampSize + 1 + // +1 for space
#endif
        facility.size() + 1 + logLevel.size() + 1 + taskInfoSize + 2 +
        formatSize + 1; // +1 for '.' between facility and log level,
                        // +1 for space, +2 for ": ", +1 for newline

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

    // -----------------------------------
    // FORMAT
    // -----------------------------------

    // Timestamp
#if MCU_LOG_TIMESTAMP_ENABLE
    std::format_to(&activeBuffer[g_bufferOffset], TIMESTAMP_FORMAT, timestamp);
    g_bufferOffset += timestampSize;
    activeBuffer[g_bufferOffset++] = ' ';
#endif
    // Facility
    std::memcpy(&activeBuffer[g_bufferOffset], facility.data(),
                facility.size());
    g_bufferOffset += facility.size();
    activeBuffer[g_bufferOffset++] = '.';

    // Log level
    std::memcpy(&activeBuffer[g_bufferOffset], logLevel.data(),
                logLevel.size());
    g_bufferOffset += logLevel.size();
    activeBuffer[g_bufferOffset++] = ' ';

    // Task info
    std::format_to(&activeBuffer[g_bufferOffset], TASK_INFO_FORMAT, taskName,
                   taskNumber);
    g_bufferOffset += taskInfoSize;
    std::memcpy(&activeBuffer[g_bufferOffset], ": ", 2);
    g_bufferOffset += 2;

    // Message
    std::format_to(&activeBuffer[g_bufferOffset], format,
                   std::forward<Args>(args)...);
    g_bufferOffset += formatSize;

    // Newline
    activeBuffer[g_bufferOffset++] = '\n';
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
/// @param facility Facility string (e.g., "main").
/// @param format Format string compatible with std::format.
/// @param ... Any arguments to format.
#define mcu_log_debug(facility, format, ...)                                   \
  ::mcu::log::logf(facility, ::mcu::log::DEBUG_STRING,                         \
                   format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_debug(facility, format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_INFO
/// @brief Logs a message at INFO level.
/// Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_INFO.
/// @param facility Facility string (e.g., "main").
/// @param format Format string compatible with std::format.
/// @param ... Any arguments to format.
#define mcu_log_info(facility, format, ...)                                    \
  ::mcu::log::logf(facility, ::mcu::log::INFO_STRING,                          \
                   format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_info(facility, format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_WARNING
/// @brief Logs a message at WARNING level.
/// Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_WARNING.
/// @param facility Facility string (e.g., "main").
/// @param format Format string compatible with std::format.
/// @param ... Optional arguments to format.
#define mcu_log_warning(facility, format, ...)                                 \
  ::mcu::log::logf(facility, ::mcu::log::WARNING_STRING,                       \
                   format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_warning(facility, format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_ERROR
/// @brief Logs a message at ERROR level.
/// Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_ERROR.
/// @param facility Facility string (e.g., "[GPS]").
/// @param format Format string compatible with std::format.
/// @param ... Optional arguments to format.
#define mcu_log_error(facility, format, ...)                                   \
  ::mcu::log::logf(facility, ::mcu::log::ERROR_STRING,                         \
                   format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_error(facility, format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_CRITICAL
/// @brief Logs a message at CRITICAL level.
/// Compiled and executed only if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_CRITICAL.
/// @param facility Facility string (e.g., "[GPS]").
/// @param format Format string compatible with std::format.
/// @param ... Optional arguments to format.
#define mcu_log_critical(facility, format, ...)                                \
  ::mcu::log::logf(facility, ::mcu::log::CRITICAL_STRING,                      \
                   format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_critical(facility, format, ...)
#endif

namespace mcu::telemetry
{
constexpr Manager::Callback LOG_CALLBACK = {
    nullptr,
    [](mcu::telemetry::FlightStatus status, void* context) {
      mcu_log_info("telemetry", "Flight status: {}", (int)status);
    },
    [](bool connected, float rssi, float snr, void* context) {
      mcu_log_info("telemetry", "Link status: {}, RSSI: {}, SNR: {}", connected,
                   rssi, snr);
    },
    [](float roll, float pitch, float yaw, void* context) {
      mcu_log_info("telemetry", "Attitude: ({:.3f}d, {:.3f}d, {:.3f}d)", roll,
                   pitch, yaw);
    },
    [](double latitude, double longitude, void* context) {
      mcu_log_info("telemetry", "Map position: ({:.6f}d, {:.6f}d)", latitude,
                   longitude);
    },
    [](float ax, float ay, float az, float gx, float gy, float gz,
       void* context) {
      mcu_log_info("telemetry",
                   "Acceleration: ({:.3f}g, {:.3f}g, {:.3f}g), "
                   "Gyroscope: ({:.3f}d/s, {:.3f}d/s, {:.3f}d/s)",
                   ax, ay, az, gx, gy, gz);
    },
    [](float altitude, void* context) {
      mcu_log_info("telemetry", "Altitude: {:.3f}m", altitude);
    },
    [](float verticalVelocity, void* context) {
      mcu_log_info("telemetry", "Vertical velocity: {:.3f}m/s",
                   verticalVelocity);
    },
    [](float pressure1, float pressure2, void* context) {
      mcu_log_info("telemetry", "Pressure readings: ({:.3f}mbar, {:.3f}mbar)",
                   pressure1, pressure2);
    },
    [](float temperature1, float temperature2, void* context) {
      mcu_log_info("telemetry", "Temperature readings: ({:.3f}C, {:.3f}C)",
                   temperature1, temperature2);
    },
    [](float extension, void* context) {
      mcu_log_info("telemetry", "Airbrake extension: {:.3f}%",
                   extension * 100.0f);
    }};
} // namespace mcu::telemetry