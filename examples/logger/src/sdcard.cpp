#include <Arduino.h>
#include <FS.h>      // Necessario per l'ESP32 File System
#include <SD_MMC.h>  // Libreria hardware SDIO per ESP32

#include "sdcard.h"


#define MAX_STREAMS 8

typedef struct {
	const char *name;
	File file;
} csv_stream_t; // struttura dello stream sarebbe quindi un file

static csv_stream_t streams[MAX_STREAMS]; // assurdo
static int num_streams = 0;
static int current_session = -1; // perche' poi la incrementa
static char session_dir[32];

static int inc_session_number()
{
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
	if (!wf) return -1;
	wf.print(num);
	wf.close();

	return num;
}


bool sdcard_init(void)
{
	// MODIFICA: SD_MMC non usa il chip select (CS). Usa i pin dedicati dell'ESP32.
	// Nota: Se usi schede che cablano solo a 1-bit, usa SD_MMC.begin("/sdcard", true)
	// TODO: define these pins
	SD_MMC.setPins(46, 47, 48);
	if (!SD_MMC.begin("/sdcard", true)) return false;

	if (!SD_MMC.exists("/session_num.txt")) {
		File f = SD_MMC.open("/session_num.txt", FILE_WRITE);
		if (!f) return false;
		f.print(0);
		f.close();
	}

	return true;
}


bool sdcard_start_session(void)
{
	int n = inc_session_number();
	if (n < 0) return false;

	current_session = n;

	// MODIFICA: Aggiunto lo slash "/" per creare correttamente la directory nella root
	snprintf(session_dir, sizeof(session_dir), "/session_%d", n);
	SD_MMC.mkdir(session_dir);

	num_streams = 0;

	return true;
}


bool sdcard_end_session(void)
{
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
	snprintf(path, sizeof(path), "%s/%s.csv", session_dir, desc->message_name);

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
  char buf[32]; // Consiglio: allargato da 16 a 32 byte per gli uint64 più lunghi
  for (uint8_t i = 0; i < desc->num_fields; i++) {
	if (i > 0)
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


// Funzione per loggare testo libero in un file "eventi.txt" della sessione corrente
bool sdcard_log_text(const char* text_message)
{
	if (current_session < 0) return false; // Se non c'è una sessione attiva, annulla

	char path[64];
	// Crea il percorso, es: /session_5/eventi.txt
	snprintf(path, sizeof(path), "%s/eventi.txt", session_dir);

	// Apre il file in modalità APPEND (aggiunge in coda senza cancellare)
	File f = SD_MMC.open(path, FILE_APPEND);
	if (!f) return false;

	// Scrive il messaggio e va a capo, poi chiude subito per salvare i dati
	f.println(text_message);
	f.close();

	return true;
}
