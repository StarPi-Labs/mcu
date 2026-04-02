#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>
#include <cstdio>
#include <cstring>
#include <format>
#include <functional>
#include <string_view>

#ifndef MCU_LOG_BUFFER_SIZE
// Dimensione del buffer per i messaggi di log.
// NOTA: Deve essere almeno grande quanto il prefisso più lungo.
#define MCU_LOG_BUFFER_SIZE 8192
#endif

// Livelli di log
#define MCU_LOG_LEVEL_DEBUG 0
#define MCU_LOG_LEVEL_INFO 1
#define MCU_LOG_LEVEL_WARNING 2
#define MCU_LOG_LEVEL_ERROR 3
#define MCU_LOG_LEVEL_CRITICAL 4
#define MCU_LOG_LEVEL_NONE 5

// Prefissi
#define MCU_LOG_PREFIX_DEBUG "[DEBUG]"
#define MCU_LOG_PREFIX_INFO "[INFO]"
#define MCU_LOG_PREFIX_WARNING "[WARNING]"
#define MCU_LOG_PREFIX_ERROR "[ERROR]"
#define MCU_LOG_PREFIX_CRITICAL "[CRITICAL]"

#if DEBUG || RELEASE
// Livello di log predefinito
#define MCU_LOG_LEVEL MCU_LOG_LEVEL_DEBUG
#else // PRODUCTION
#define MCU_LOG_LEVEL MCU_LOG_LEVEL_INFO
#endif

// Accesso sicuro agli array per evitare out-of-bounds in fase di debug/release
#if DEBUG || RELEASE
#define array_access(array, index) ((array).at(index))
#else // Production
#define array_access(array, index) ((array)[(index)])
#endif

namespace mcu {
// Buffer di log
inline struct {
  // Dati del messaggio di log formattato, incluso il prefisso
  std::array<char, MCU_LOG_BUFFER_SIZE> data;
  // Lunghezza attuale del messaggio nel buffer, escluso il terminatore di stringa
  std::size_t length = 0;
} g_logBuffer;

namespace log_handler {
// Interfaccia per gli handler di log.
typedef std::function<void(const char *)> LogHandler_t;

const LogHandler_t ARDUINO_SERIAL = [](const char *msg) { Serial.print(msg); };
} // namespace log_handler

// Gli handler di log verso cui mandare il messaggio dopo
// la formattazione
inline std::vector<log_handler::LogHandler_t> g_logHandlers;

/**
 * @brief Logga (stampa) un messaggio formattato con un prefisso.
 *
 * Non è pensata per essere usata direttamente, ma tramite le macro:
 * mcu_log_debug, mcu_log_info, mcu_log_warning, mcu_log_error,
 mcu_log_critical.

 * @param logLevel La stringa del livello di log (es. "[DEBUG]").
 * @param format La stringa di formato (usa std::format_string,
 * per info sulla sintassi @see
 https://en.cppreference.com/w/cpp/utility/format/spec.html).
 * @param args... Gli argomenti variabili da formattare nella stringa.
 */
template <typename... Args>
inline void
logf(const std::string_view &logLevel,
     const std::format_string<std::type_identity_t<Args>...> &format,
     Args &&...args) {
  // Scrive il prefisso nel buffer
  std::memcpy(&array_access(g_logBuffer.data, g_logBuffer.length), logLevel.data(),
              logLevel.size());

  g_logBuffer.length += logLevel.size();

  // Formatta il messaggio nel buffer
  std::format_to_n_result formatResult =
      std::format_to_n(&array_access(g_logBuffer.data, g_logBuffer.length),
                       g_logBuffer.data.size() - g_logBuffer.length - 1, format,
                       std::forward<Args>(args)...);

  *formatResult.out = '\0'; // Aggiunge il terminatore di stringa
  g_logBuffer.length += formatResult.size;
}

/**
 * @brief Task FreeRTOS per gestire la stampa dei messaggi di log.
 *
 * Questo task si occupa di prendere i messaggi formattati da `g_logBuffer`
 * e inviarli a tutti gli handler registrati in `g_logHandlers`.
 *
 * @param pvParams Parametri del task (non usati in questo caso).
 */
void vTaskLogger(void *pvParams);
} // namespace mcu

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_DEBUG
#define mcu_log_debug(format, ...)                                             \
  mcu::logf(                                                                   \
      std::string_view(MCU_LOG_PREFIX_DEBUG, sizeof(MCU_LOG_PREFIX_DEBUG) - 1),    \
      format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_debug(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_INFO
#define mcu_log_info(format, ...)                                              \
  mcu::logf(                                                                   \
      std::string_view(MCU_LOG_PREFIX_INFO, sizeof(MCU_LOG_PREFIX_INFO) - 1),      \
      format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_info(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_WARNING
#define mcu_log_warning(format, ...)                                           \
  mcu::logf(std::string_view(MCU_LOG_PREFIX_WARNING,                           \
                             sizeof(MCU_LOG_PREFIX_WARNING) - 1),                  \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_warning(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_ERROR
#define mcu_log_error(format, ...)                                             \
  mcu::logf(                                                                   \
      std::string_view(MCU_LOG_PREFIX_ERROR, sizeof(MCU_LOG_PREFIX_ERROR) - 1),    \
      format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_error(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_CRITICAL
#define mcu_log_critical(format, ...)                                          \
  mcu::logf(std::string_view(MCU_LOG_PREFIX_CRITICAL,                          \
                             sizeof(MCU_LOG_PREFIX_CRITICAL) - 1),                 \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_critical(format, ...)
#endif

#endif // LOGGING_H
