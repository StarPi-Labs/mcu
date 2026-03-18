#include "barometro.h"
#include <Arduino.h>


void setup() {
    Serial.begin(115200);// da settare baudrate del pc uguale
    Serial.println("Avvio Sistema Avionico...");
    
    // inizializzo modulo
    if (initBarometro()) {
        // Se l'init va a buon fine, avvia il task. assegno 2048 byte, con priorità 3 (alta)
        xTaskCreate(vTaskBarometro, "TaskBaro", 2048, NULL, 3, NULL);
        
        // Avvia il task del team (Logger)
        xTaskCreate(vTaskDatalogger, "TaskLog", 2048, NULL, 2, NULL);
    } else {
        Serial.println("Avvio interrotto causa guasto sensore.");
    }
}

void loop() {
    // Vuoto in FreeRTOS
}