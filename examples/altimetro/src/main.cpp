#include <Arduino.h>
#include "barometro.h"
#include "imu.h"

//La funzione definisce cosa stampare a schermo una volta letti i dati inseriti nelle code
void vTaskDatalogger(void *pvParameters) {
    DatiBarometro datiRicevuti;
    DatiIMU datiIMU;
    for(;;) {
        // Aspetta finché non arriva un nuovo dato dal barometro
        if (xQueueReceive(codaBarometro, &datiRicevuti, portMAX_DELAY) == pdPASS) {
            Serial.printf("T: %lu | Alt: %.2f m | Press: %.2f mbar | Temp: %.2f C\n", 
                          datiRicevuti.timestamp, 
                          datiRicevuti.altitudine, 
                          datiRicevuti.pressione,
                          datiRicevuti.temperatura);
        }
        if (xQueueReceive(codaIMU, &datiIMU, portMAX_DELAY) == pdPASS) {
            Serial.printf("T: %lu | Acc: (%.2f, %.2f, %.2f) m/s^2 | Gyro: (%.2f, %.2f, %.2f) deg/s\n", 
                          datiIMU.timestamp, 
                          datiIMU.acc_x, datiIMU.acc_y, datiIMU.acc_z,
                          datiIMU.gyro_x, datiIMU.gyro_y, datiIMU.gyro_z);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Avvio Sistema Avionico...");
    
    // inizializzo modulo
    if (initBarometro()) {
        // Se l'init va a buon fine, avvia il task. assegno 2048 byte, con priorità 3 (alta)
        xTaskCreate(vTaskBarometro, "TaskBaro", 2048, NULL, 3, NULL);
    } else {
        Serial.println("Avvio interrotto causa guasto sensore.");
    }

    if (initIMU()) {
        // Se l'init va a buon fine, avvia il task. assegno 2048 byte, con priorità 3 (alta)
        xTaskCreate(vTaskIMU, "TaskIMU", 2048, NULL, 3, NULL);
    } else {
        Serial.println("Avvio interrotto causa guasto sensore.");
    }

    // Avvia il task del team (Logger)
    xTaskCreate(vTaskDatalogger, "TaskLog", 2048, NULL, 2, NULL);
}

void loop() {
    // Vuoto in FreeRTOS
}