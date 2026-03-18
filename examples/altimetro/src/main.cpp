#include <Arduino.h>
#include "barometro.h"

void vTaskDatalogger(void *pvParameters) {
    DatiBarometro datiRicevuti;
    for(;;) {
        // Aspetta finché non arriva un nuovo dato dal barometro
        if (xQueueReceive(codaBarometro, &datiRicevuti, portMAX_DELAY) == pdPASS) {
            Serial.printf("T: %lu | Alt: %.2f m | Press: %.2f mbar | Temp: %.2f C\n", 
                          datiRicevuti.timestamp, 
                          datiRicevuti.altitudine, 
                          datiRicevuti.pressione,
                          datiRicevuti.temperatura);
        }
    }
}

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