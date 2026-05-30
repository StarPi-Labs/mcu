//MODIFICARE TUTTI I SERIAL PER LA COMUNICAZIONE E IL SALVATAGGIO SU SD
//DECOMMENTARE I TASK DEI SENSORI QUANDO IMPLEMENTATI

#include <Arduino.h>
#include "dati_volo.h"
#include "paracadute.h"

QueueHandle_t codaIMU;
QueueHandle_t codaBarometro;
QueueHandle_t codaLogger;


void setup() {
    Serial.begin(115200);
    Serial.println("--- AVVIO SISTEMA PARACADUTE ---");

    codaIMU = xQueueCreate(10, sizeof(DatiIMU));
    codaBarometro = xQueueCreate(10, sizeof(DatiBarometro));
    codaLogger = xQueueCreate(20, sizeof(PacchettoLog));

    //Avvio task sensori
    // xTaskCreate(vTaskIMU, "TaskIMU", 2048, NULL, 3, NULL);
    // xTaskCreate(vTaskBarometro, "TaskBaro", 2048, NULL, 3, NULL);

    //Avvio task macchina a stati
    xTaskCreate(vTaskPara, "TaskFSM", 4096, NULL, 2, NULL);

    //Avvio task datalogger
    xTaskCreate(vTaskDatalogger, "TaskLog", 4096, NULL, 1, NULL);
}

void loop() {
}