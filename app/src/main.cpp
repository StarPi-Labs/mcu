#include "barometro.h"
#include <Arduino.h>

namespace mcu {
    bool init();
};

void setup() {
  Serial.begin(115200); // da settare baudrate del pc uguale
  Serial.println("[Info]: Avvio Sistema Avionico...");

  if (!mcu::init()) {
    Serial.print("[Errore]: Inizializzazione fallita");
    return;
  }

  Serial.println("[Info]: Inizialiazzazione completata con successo");
}

bool mcu::init() {
  if (!initBarometro()) {
    Serial.println("Avvio interrotto causa guasto sensore.");
    return false;
  }

  // Se l'init va a buon fine, avvia il task. assegno 2048 byte, con priorità 3
  // (alta)
  xTaskCreate(vTaskBarometro, "TaskBaro", 2048, NULL, 3, NULL);
  // Avvia il task del team (Logger)
  xTaskCreate(vTaskDatalogger, "TaskLog", 2048, NULL, 2, NULL);

  return true;
}

void loop() {
  // Vuoto in FreeRTOS
}
