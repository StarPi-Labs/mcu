#include <Arduino.h>
#include "barometro.h"
#include "imu.h"
#include "dati_volo.h"
#include "paracadute.h"

QueueHandle_t codaLogger;

// La funzione definisce cosa stampare a schermo una volta letti i dati unificati
void vTaskDatalogger(void *pvParameters) {
    PacchettoLog logRicevuto;
    
    for(;;) {
        if (xQueueReceive(codaLogger, &logRicevuto, portMAX_DELAY) == pdPASS) {
            
            Serial.printf("T: %lu | Stato: %d | Alt: %.2f m | Press: %.2f mbar | AccZ: %.2f m/s^2 | GyroZ: %.2f deg/s\n", 
                          logRicevuto.timestamp,
                          static_cast<int>(logRicevuto.stato),
                          logRicevuto.baro.altitudine, 
                          logRicevuto.baro.pressione,
                          logRicevuto.imu.acc_z,
                          logRicevuto.imu.gyro_z);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Avvio Sistema Avionico...");

    // 1. CREAZIONE CODA PER IL LOGGER
    codaLogger = xQueueCreate(20, sizeof(PacchettoLog));

    // 2. INIZIALIZZAZIONE SENSORI E AVVIO TASK (Priorità 3)
    if (initBarometro()) {
        xTaskCreate(vTaskBarometro, "TaskBaro", 2048, NULL, 3, NULL);
    } else {
        Serial.println("ERRORE: Avvio interrotto causa guasto sensore Barometro.");
    }

    if (initIMU()) {
        xTaskCreate(vTaskIMU, "TaskIMU", 2048, NULL, 3, NULL);
    } else {
        Serial.println("ERRORE: Avvio interrotto causa guasto sensore IMU.");
    }

    // 3. AVVIO TASK MACCHINA A STATI E PARACADUTE (Priorità 2)
    xTaskCreate(vTaskPara, "TaskFSM", 4096, NULL, 2, NULL);

    // 4. AVVIO TASK DATALOGGER (Priorità 1)
    xTaskCreate(vTaskDatalogger, "TaskLog", 4096, NULL, 1, NULL);
}

void loop() {}
