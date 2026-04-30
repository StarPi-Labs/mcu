#include <Arduino.h>
#include "dati_volo.h"
#include "paracadute.h"

QueueHandle_t codaIMU;
QueueHandle_t codaBarometro;
QueueHandle_t codaLogger;


void vTaskDatalogger(void *pvParameters) {
    PacchettoLog logRicevuto;
    for(;;) {
        if (xQueueReceive(codaLogger, &logRicevuto, portMAX_DELAY) == pdPASS) {  
            Serial.printf("T: %lu | Stato: %d | Alt: %.1f m | AccZ: %.1f g | VelZ: %.1f m/s\n", 
                          logRicevuto.timestamp, 
                          static_cast<int>(logRicevuto.stato),
                          logRicevuto.baro.altitudine, 
                          logRicevuto.imu.acc_z, 
                          logRicevuto.imu.vel_z);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("--- AVVIO SISTEMA AVIONICO (MODULO PARACADUTE) ---");

    codaIMU = xQueueCreate(10, sizeof(DatiIMU));
    codaBarometro = xQueueCreate(10, sizeof(DatiBarometro));
    codaLogger = xQueueCreate(20, sizeof(PacchettoLog));

    //Avvio task sensori (DA DECOMMENTARE QUANDO IMPLEMENTATI)
    // xTaskCreate(vTaskIMU, "TaskIMU", 2048, NULL, 3, NULL);
    // xTaskCreate(vTaskBarometro, "TaskBaro", 2048, NULL, 3, NULL);

    //Avvio task macchina a stati
    xTaskCreate(vTaskPara, "TaskFSM", 4096, NULL, 2, NULL);

    //Avvio task datalogger
    xTaskCreate(vTaskDatalogger, "TaskLog", 4096, NULL, 1, NULL);
}

void loop() {
}