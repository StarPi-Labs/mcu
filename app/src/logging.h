#ifndef LOGGING_H
#define LOGGING_H

#include <string_view>

#ifndef MCU_MAX_LOG_BUFFER_SIZE
// Dimensione massima del buffer per i messaggi di log.
// NOTA: Deve essere almeno grande quanto il prefisso più lungo.
#define MCU_MAX_LOG_BUFFER_SIZE 1024
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

namespace mcu {
/**
 * @brief Logga (stampa) un messaggio formattato con un prefisso.
 * Non è pensata per essere usata direttamente, ma tramite le macro:
 * - mcu_log_debug
 * - mcu_log_info
 * - mcu_log_warning
 * - mcu_log_error
 * - mcu_log_critical

 * @param prefix Il prefisso da aggiungere all'inizio del messaggio (es.
 "[DEBUG]: ").
 * @param prefix_length La lunghezza del prefisso (incluso il terminatore '\0').
 * @param format La stringa di formato (simile a printf).
 * @param ... Gli argomenti variabili da formattare nella stringa (come printf).
 */
void logf(const char *prefix, size_t prefix_length, const char *in_format, ...);
} // namespace mcu

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_DEBUG
#define mcu_log_debug(format, ...)                                             \
  mcu::logf(MCU_LOG_PREFIX_DEBUG, sizeof(MCU_LOG_PREFIX_DEBUG),                \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_debug(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_INFO
#define mcu_log_info(format, ...)                                              \
  mcu::logf(MCU_LOG_PREFIX_INFO, sizeof(MCU_LOG_PREFIX_INFO),                  \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_info(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_WARNING
#define mcu_log_warning(format, ...)                                           \
  mcu::logf(MCU_LOG_PREFIX_WARNING, sizeof(MCU_LOG_PREFIX_WARNING),            \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_warning(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_ERROR
#define mcu_log_error(format, ...)                                             \
  mcu::logf(MCU_LOG_PREFIX_ERROR, sizeof(MCU_LOG_PREFIX_ERROR),                \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_error(format, ...)
#endif

#if MCU_LOG_LEVEL <= MCU_LOG_LEVEL_CRITICAL
#define mcu_log_critical(format, ...)                                          \
  mcu::logf(MCU_LOG_PREFIX_CRITICAL, sizeof(MCU_LOG_PREFIX_CRITICAL),          \
            format __VA_OPT__(, ) __VA_ARGS__)
#else
#define mcu_log_critical(format, ...)
#endif

#endif // LOGGING_H
