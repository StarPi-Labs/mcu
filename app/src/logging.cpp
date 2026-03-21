#include "logging.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

// Dimensione massima del buffer per i messaggi di log.
// NOTA: Deve essere almeno grande quanto il prefisso più lungo.
#define MAX_LOG_BUFFER_SIZE 1024

#define PREFIX_DEBUG "[DEBUG]: "
#define PREFIX_INFO "[INFO]: "
#define PREFIX_WARNING "[WARNING]: "
#define PREFIX_ERROR "[ERROR]: "
#define PREFIX_UNSUPPORTED "[UNSUPPORTED]: "


/**
 * @brief Funzione helper per ottenere il prefisso in base al livello di log.
 *
 * @param level Il livello di log.
 * @param out_buffer Il buffer in cui verrà scritto il prefisso.
 *
 * @return La lunghezza del prefisso scritto nel buffer (escluso il terminatore
 * '\0').
 */
static size_t write_prefix(mcu::LogLevel level, char *out_buffer);

void mcu::logf(mcu::LogLevel log_level, const char *format, ...) {
  // Prealloca un buffer
  char buffer[MAX_LOG_BUFFER_SIZE];

  // Scrivi il prefisso nel buffer e ottieni la lunghezza del prefisso
  size_t prefix_length = write_prefix(log_level, &buffer[0]);

  // Formatta il messaggio dopo il prefisso
  va_list args;
  va_start(args, format);
  // buffer[prefix_length - 1] è '\0' dopo write_prefix (sovrascrivibile).
  size_t format_written =
      std::vsnprintf(&buffer[prefix_length - 1],
                     MAX_LOG_BUFFER_SIZE - prefix_length + 1, format, args);
  va_end(args);

  if (format_written < 0)
    // Errore nella formattazione, scrivi un messaggio di errore
    std::strncpy(&buffer[prefix_length - 1],
                 "[LOGGING ERROR: Formattazione fallita]",
                 MAX_LOG_BUFFER_SIZE - prefix_length + 1);

  Serial.print(buffer);
}

static size_t write_prefix(mcu::LogLevel level, char out_buffer[]) {
  switch (level) {
  case mcu::LogLevel::DEBUG:
    std::strcpy(out_buffer, PREFIX_DEBUG);
    return sizeof(PREFIX_DEBUG);
    break;
  case mcu::LogLevel::INFO:
    std::strcpy(out_buffer, PREFIX_INFO);
    return sizeof(PREFIX_INFO);
    break;
  case mcu::LogLevel::WARNING:
    std::strcpy(out_buffer, PREFIX_WARNING);
    return sizeof(PREFIX_WARNING);
    break;
  case mcu::LogLevel::ERROR:
    std::strcpy(out_buffer, PREFIX_ERROR);
    return sizeof(PREFIX_ERROR);
    break;
  default:
    std::strcpy(out_buffer, PREFIX_UNSUPPORTED);
    return sizeof(PREFIX_UNSUPPORTED);
  }
}
