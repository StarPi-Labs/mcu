#include "logging.h"

#include <Arduino.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#define MCU_LOG_FORMATTING_ERROR "[ERROR]: Error formatting log message.\n"
#define MCU_LOG_TRUNCATION_SUFFIX " [TRUNCATED]\n"

#if DEBUG || RELEASE
#define array_access(array, index) ((array).at(index))
#else
#define array_access(array, index) ((array)[(index)])
#endif

static void handle_format_result(
    std::array<char, MCU_LOG_BUFFER_SIZE> buffer, size_t prefix_length,
    int written) {
  if (written < 0) {
    std::memcpy(buffer.data(), MCU_LOG_FORMATTING_ERROR,
                sizeof(MCU_LOG_FORMATTING_ERROR));
    return;
  }

  if (written > MCU_LOG_BUFFER_SIZE - prefix_length) {
    // Se il messaggio viene troncato perché troppo lungo
    //
    //                                     |+-- MAX_LOG_BUFFER_SIZE
    // e.g. "[INFO]: This is going to be a"
    //                      ^
    //                      |+-- MCU_LOG_BUFFER_SIZE - sizeof(" [TRUNCATED]\n")
    //                      |
    //                     " [TRUNCATED]\n"
    //
    std::memcpy(
        &array_access(buffer, MCU_LOG_BUFFER_SIZE - sizeof(MCU_LOG_TRUNCATION_SUFFIX)),
        MCU_LOG_TRUNCATION_SUFFIX,
        sizeof(MCU_LOG_TRUNCATION_SUFFIX) - 1); // -1 perché c'è già '\0'
  }
}

void mcu::logf(const std::string_view &prefix, const char *format, ...) {
  // TODO: mcu_assert(prefix != nullptr, "[ERROR]: Log prefix cannot be null.");
  // TODO: mcu_assert(prefix_length < MCU_LOG_BUFFER_SIZE,
  // "[ERROR]: Log prefix length must be between 1 and
  // MCU_LOG_BUFFER_SIZE.");
  // TODO: mcu_assert(format != nullptr, "[ERROR]: Log format string cannot be
  // null.");

  // Uso un buffer statico per evitare di allocare memoria dinamicamente su
  // stack/heap
  thread_local static std::array<char, MCU_LOG_BUFFER_SIZE> buffer;

  std::memcpy(buffer.data(), prefix.data(), prefix.size());

  // Formatta il messaggio
  va_list args;
  va_start(args, format);
  int written =
      std::vsnprintf(&array_access(buffer, prefix.size() - 1),
                     MCU_LOG_BUFFER_SIZE - prefix.size() + 1, format, args);
  va_end(args);

  handle_format_result(buffer, prefix.size(), written);

  Serial.print(buffer.data());
}
