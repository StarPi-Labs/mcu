#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>

namespace mcu {
// Livelli di logging
enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

/**
 * @brief Logga (stampa) il messaggio formattato con il livello di log
 * specificato.
 * @param log_level Il livello di log (DEBUG, INFO, WARNING, ERROR).
 * @param format La stringa di formato (simile a printf).
 * @param ... Gli argomenti variabili da formattare nella stringa.
 */
void logf(LogLevel log_level, const char *format, ...);
} // namespace mcu

#endif // LOGGING_H
