#include <Arduino.h>
#include <FS.h>     // Necessario per l'ESP32 File System
#include <SD_MMC.h> // Libreria hardware SDIO per ESP32
#include <string.h>

#include "sdcard.h"
#include <task.h>

// Double-buffer state for non-blocking SD writes
enum { BUF_FREE, BUF_PENDING, BUF_FLUSHING };

typedef struct {
  char data[SD_BUF_SIZE];
  uint32_t len;
  int state;
} sd_buf_t;

static sd_buf_t bufs[2];
static int active_idx; // buffer being filled by sdcard_write()
static int flush_idx;  // buffer being flushed to file (-1 = none)

DECLARE_STATIC_SEMAPHORE(buf_lock);

#define MAX_STREAMS 8

typedef struct {
  const char *name;
  File file;
} csv_stream_t; // struttura dello stream sarebbe quindi un file

static csv_stream_t streams[MAX_STREAMS]; // assurdo
static int num_streams = 0;
static int current_session = -1; // perche' poi la incrementa
static char session_dir[32];
static char log_path[64];
static File log_file;

static int inc_session_number() {
  int num = 0;

  // MODIFICA: Aggiunto lo slash "/" obbligatorio per ESP32
  File f = SD_MMC.open("/session_num.txt", FILE_READ);

  if (f) {
    num = f.readString().toInt();
    f.close();
  }
  if (num < 0)
    num = 0;
  num++;

  SD_MMC.remove("/session_num.txt");

  // Sull'ESP32 FILE_WRITE sovrascrive il file (lo tronca a zero),
  // a differenza della vecchia libreria SD dove accodava (append).
  File wf = SD_MMC.open("/session_num.txt", FILE_WRITE);
  if (!wf)
    return -1;
  wf.print(num);
  wf.close();

  return num;
}

bool sdcard_init(void) {
  // MODIFICA: SD_MMC non usa il chip select (CS). Usa i pin dedicati
  // dell'ESP32. Nota: Se usi schede che cablano solo a 1-bit, usa
  // SD_MMC.begin("/sdcard", true)
  // TODO: define these pins
  SD_MMC.setPins(46, 47, 48);
  if (!SD_MMC.begin("/sdcard", true, false, 40000))
    return false;

  if (!SD_MMC.exists("/session_num.txt")) {
    File f = SD_MMC.open("/session_num.txt", FILE_WRITE);
    if (!f)
      return false;
    f.print(0);
    f.close();
  }

  // Initialize double buffer
  INIT_STATIC_SEMAPHORE(buf_lock);
  if (buf_lock == NULL)
    return false;

  bufs[0].len = 0;
  bufs[0].state = BUF_FREE;
  bufs[1].len = 0;
  bufs[1].state = BUF_FREE;
  active_idx = 0;
  flush_idx = -1;

  return true;
}

bool sdcard_start_session(void) {
  int n = inc_session_number();
  if (n < 0)
    return false;

  current_session = n;

  // Crea la cartella della sessione, es: /session_5
  snprintf(session_dir, sizeof(session_dir), "/session_%d", n);
  SD_MMC.mkdir(session_dir);

  // Crea il file di log degli eventi, es: /session_5/eventi.txt
  snprintf(log_path, sizeof(log_path), "%s/eventi.txt", session_dir);
  File f = SD_MMC.open(log_path, FILE_WRITE);
  if (!f)
    return false;
  f.close();

  num_streams = 0;

  return true;
}

bool sdcard_end_session(void) {
  for (int i = 0; i < num_streams; i++) {
    streams[i].file.flush();
    streams[i].file.close();
  }
  num_streams = 0;
  current_session = -1;
  return true;
}

/*
bool sdcard_log(const void *message, const MessageDescriptor *desc) {
        if (current_session < 0) return false;

  // Find existing stream for this message type
  csv_stream_t *s = nullptr;
  for (int i = 0; i < num_streams; i++) {
        if (strcmp(streams[i].name, desc->message_name) == 0) {
          s = &streams[i];
          break;
        }
  }

  // Lazy-open: create new stream entry
  if (s == nullptr) {
        if (num_streams >= MAX_STREAMS)
          return false;

        char path[64];
        // session_dir ha già lo slash iniziale ("/session_X/Nome.csv")
        snprintf(path, sizeof(path), "%s/%s.csv", session_dir,
desc->message_name);

        File f = SD_MMC.open(path, FILE_WRITE);
        if (!f)
          return false;

        // Write CSV header
        for (uint8_t i = 0; i < desc->num_fields; i++) {
          if (i > 0)
                f.print(',');
          f.print(desc->field_name[i]);
        }
        f.print('\n');

        streams[num_streams].name = desc->message_name;
        streams[num_streams].file = f;
        s = &streams[num_streams];
        num_streams++;
  }

  // Write CSV data row
  char buf[32]; // Consiglio: allargato da 16 a 32 byte per gli uint64 più
lunghi for (uint8_t i = 0; i < desc->num_fields; i++) { if (i > 0)
          s->file.print(',');
        if (desc->field_type[i] == FIELD_TYPE_UINT64) {
          snprintf(buf, sizeof(buf), "%llu",
                           message_field_as_uint64(message, desc, i));
        } else {
          snprintf(buf, sizeof(buf), "%d",
                           message_field_as_int32(message, desc, i));
        }
        s->file.print(buf);
  }
  s->file.print('\n');

  return true;
}
*/

bool sdcard_open_log(void) {
  if (current_session < 0)
    return false;
  log_file = SD_MMC.open(log_path, FILE_APPEND);
  if (!log_file)
    return false;
  return true;
}

bool sdcard_close_log(void) {
  if (current_session < 0)
    return false;
  if (log_file) {
    log_file.close();
    return true;
  }
  return false;
}

// Funzione per loggare testo libero in un file "eventi.txt" della sessione
// corrente
bool sdcard_log_text(const char *text_message) {
  if (!sdcard_open_log())
    return false;

  // Scrive il messaggio e va a capo, poi chiude subito per salvare i dati
  log_file.println(text_message);

  return sdcard_close_log();
}

bool sdcard_write_str(const char *str) {
  if (!log_file)
    return false;
  log_file.println(str);
  return true;
}

bool sdcard_flush(void) {
  if (!log_file)
    return false;

  if (xSemaphoreTake(buf_lock, portMAX_DELAY) != pdTRUE)
    return false;

  // If no buffer is pending, mark the active buffer as pending first
  if (flush_idx < 0 && bufs[active_idx].len > 0) {
    bufs[active_idx].state = BUF_PENDING;
    flush_idx = active_idx;
    active_idx = !active_idx;
    bufs[active_idx].len = 0;
    bufs[active_idx].state = BUF_FREE;
  }

  int idx = flush_idx;
  if (idx < 0 || bufs[idx].state != BUF_PENDING || bufs[idx].len == 0) {
    xSemaphoreGive(buf_lock);
    return true;
  }

  bufs[idx].state = BUF_FLUSHING;
  xSemaphoreGive(buf_lock);

  log_file.write((const uint8_t *)bufs[idx].data, bufs[idx].len);
  log_file.flush();
  // FIXME: flush here does not prevent losing data

  if (xSemaphoreTake(buf_lock, portMAX_DELAY) == pdTRUE) {
    bufs[idx].len = 0;
    bufs[idx].state = BUF_FREE;
    if (flush_idx == idx)
      flush_idx = -1;
    xSemaphoreGive(buf_lock);
  }

  return true;
}

bool sdcard_write(const char *buf, uint32_t len) {
  if (!buf || len == 0)
    return false;

  if (xSemaphoreTake(buf_lock, portMAX_DELAY) != pdTRUE)
    return false;

  sd_buf_t *active = &bufs[active_idx];

  // If there's no space and the other buffer is free, swap
  if (active->len + len > SD_BUF_SIZE) {
    int other = !active_idx;

    if (bufs[other].state == BUF_FREE) {
      // Mark current as pending, switch to other
      active->state = BUF_PENDING;
      flush_idx = active_idx;
      active_idx = other;
      bufs[other].len = 0;
      active = &bufs[other];
    } else {
      // Both buffers full — auto-flush the pending one
      xSemaphoreGive(buf_lock);
      sdcard_flush();
      if (xSemaphoreTake(buf_lock, portMAX_DELAY) != pdTRUE)
        return false;
      active = &bufs[active_idx];
      if (active->len + len > SD_BUF_SIZE) {
        xSemaphoreGive(buf_lock);
        return false;
      }
    }
  }

  memcpy(active->data + active->len, buf, len);
  active->len += len;

  xSemaphoreGive(buf_lock);
  return true;
}