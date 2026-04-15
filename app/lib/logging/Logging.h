#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <format>
#include <functional>
#include <mutex>
#include <string_view>
#include <array>
#include <Utils.h>

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
constexpr std::string_view MCU_LOG_PREFIX_DEBUG = "[DEBUG]";
constexpr std::string_view MCU_LOG_PREFIX_INFO = "[INFO]";
constexpr std::string_view MCU_LOG_PREFIX_WARNING = "[WARNING]";
constexpr std::string_view MCU_LOG_PREFIX_ERROR = "[ERROR]";
constexpr std::string_view MCU_LOG_PREFIX_CRITICAL = "[CRITICAL]";

#if DEBUG || RELEASE
// Livello di log predefinito
#define MCU_LOG_LEVEL MCU_LOG_LEVEL_DEBUG
#else // PRODUCTION
#define MCU_LOG_LEVEL MCU_LOG_LEVEL_INFO
#endif

namespace mcu {
// Buffer di log
inline struct {
  // Dati del messaggio di log formattato, incluso il prefisso
  std::array<char, MCU_LOG_BUFFER_SIZE> data;
  // Lunghezza attuale del messaggio nel buffer, escluso il terminatore di
  // stringa
  std::size_t length = 0;
  mcu::Mutex mutex;
  std::condition_variable_any cvBufferEmpty, cvBufferFull;
} g_logBuffer;

namespace log_handler {
// Interfaccia per gli handler di log.
typedef std::function<void(std::string_view)> LogHandler_t;

const LogHandler_t ARDUINO_SERIAL = [](std::string_view msg) {
  Serial.write(reinterpret_cast<const uint8_t *>(msg.data()), msg.size());
};
} // namespace log_handler

// Gli handler di log verso cui mandare il messaggio dopo
// la formattazione, nota non è pensata per essere modificata a runtime
// ma solo in fase di inizializzazione del programma (e.g. setup())
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
  const std::size_t messageSize =
      std::formatted_size(format, std::forward<Args>(args)...);
  const std::size_t requiredSize = logLevel.size() + messageSize;

  assert(requiredSize <= MCU_LOG_BUFFER_SIZE);

  {
    std::unique_lock lock(g_logBuffer.mutex);

    g_logBuffer.cvBufferFull.wait(lock, [&, requiredSize] {
      return g_logBuffer.length + requiredSize <= MCU_LOG_BUFFER_SIZE;
    });

    // Scrive il prefisso nel buffer
    std::memcpy(&array_access(g_logBuffer.data, g_logBuffer.length),
                logLevel.data(), logLevel.size());

    g_logBuffer.length += logLevel.size();

    // Formatta il messaggio nel buffer
    std::format_to(&array_access(g_logBuffer.data, g_logBuffer.length), format,
                   std::forward<Args>(args)...);

    g_logBuffer.length += messageSize;
  }

  g_logBuffer.cvBufferEmpty.notify_one();
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

/**
 * @brief Forza la stampa immediata di tutti i messaggi di log attualmente nel buffer.
 */
void flush();
} // namespace mcu

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_DEBUG
/**
 * @brief Logga un messaggio a livello DEBUG.
 * Se abilitato, aggiunge il prefisso [DEBUG] in testa al log.
 * Compilato ed eseguito solo se MCU_LOG_LEVEL <= MCU_LOG_LEVEL_DEBUG.
 * 
 * @param format Stringa di formato compatibile con std::format.
 * @param ... Eventuali argomenti da formattare.
 */
#define mcu_log_debug(format, ...)                                             \
  mcu::logf(MCU_LOG_PREFIX_DEBUG,                                              \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_debug(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_INFO
/**
 * @brief Logga un messaggio a livello INFO.
 * Se abilitato, aggiunge il prefisso [INFO] in testa al log.
 * Compilato ed eseguito solo se MCU_LOG_LEVEL <= MCU_LOG_LEVEL_INFO.
 * 
 * @param format Stringa di formato compatibile con std::format.
 * @param ... Eventuali argomenti da formattare.
 */
#define mcu_log_info(format, ...)                                              \
  mcu::logf(MCU_LOG_PREFIX_INFO,                                               \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_info(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_WARNING
/**
 * @brief Logga un messaggio a livello WARNING.
 * Se abilitato, aggiunge il prefisso [WARNING] in testa al log.
 * Compilato ed eseguito solo se MCU_LOG_LEVEL <= MCU_LOG_LEVEL_WARNING.
 * 
 * @param format Stringa di formato compatibile con std::format.
 * @param ... Eventuali argomenti da formattare.
 */
#define mcu_log_warning(format, ...)                                           \
  mcu::logf(MCU_LOG_PREFIX_WARNING,                                            \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_warning(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_ERROR
/**
 * @brief Logga un messaggio a livello ERROR.
 * Se abilitato, aggiunge il prefisso [ERROR] in testa al log.
 * Compilato ed eseguito solo se MCU_LOG_LEVEL <= MCU_LOG_LEVEL_ERROR.
 * 
 * @param format Stringa di formato compatibile con std::format.
 * @param ... Eventuali argomenti da formattare.
 */
#define mcu_log_error(format, ...)                                             \
  mcu::logf(MCU_LOG_PREFIX_ERROR,                                              \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_error(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_CRITICAL
/**
 * @brief Logga un messaggio a livello CRITICAL.
 * Se abilitato, aggiunge il prefisso [CRITICAL] in testa al log.
 * Compilato ed eseguito solo se MCU_LOG_LEVEL <= MCU_LOG_LEVEL_CRITICAL.
 * 
 * @param format Stringa di formato compatibile con std::format.
 * @param ... Eventuali argomenti da formattare.
 */
#define mcu_log_critical(format, ...)                                          \
  mcu::logf(MCU_LOG_PREFIX_CRITICAL,                                           \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_critical(format, ...)
#endif

#endif // LOGGING_H
