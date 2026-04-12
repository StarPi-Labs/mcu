#include <Arduino.h>
#include "sdcard.h"


void setup() {
	Serial.begin(115200);
	while (!Serial) {
		delay(100);
	}

	Serial.println("Avvio sistema...");

	// 1. Inizializzazione della SD (sveglia l'hardware)
	if (!sdcard_init()) {
		Serial.println("ERRORE CRITICO: Scheda SD non trovata o danneggiata.");
		while (1) { delay(100); }
	}

	// 2. Avvio della Sessione (creerà la cartella /session_X)
	if (!sdcard_start_session()) {
		Serial.println("ERRORE: Impossibile creare la cartella di sessione sulla SD.");
		while (1) { delay(100); }
	}

	Serial.println("Sessione SD avviata con successo!");

	// 3. IL NOSTRO CIAO MONDO
	// Usiamo la funzione testuale appena aggiunta
	if (sdcard_log_text("Sistema Avviato: Ciao Vic!")) {
		Serial.println("Messaggio salvato su SD.");
	}

	// Se volessi chiudere subito tutto e smontare la SD in sicurezza:
	// sdcard_end_session();
}


void loop() {
	// Qui dentro, durante il normale funzionamento, chiameresti
	// la tua funzione sdcard_log originale passandogli i dati dei sensori

	// Esempio:
	// log_sensori();
	// sdcard_log(&miei_dati, &desc_sensori);

	// Se succede qualcosa di anomalo, logghi il testo:
	// sdcard_log_text("Attenzione: Temperatura troppo alta!");

	delay(1000);
}
