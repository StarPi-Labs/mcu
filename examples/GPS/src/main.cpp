#include <Arduino.h>
#include "gps_logger.h"

//Creazione della coda per i dati GPS
QueueHandle_t codaDatiGPS;

void setup() {
    Serial.begin(115200);
    //Inizializzazione della coda per i dati GPS
    codaDatiGPS = xQueueCreate(10, sizeof(ReportGPS));

    //Creazione dei task per la lettura GPS e il monitoraggio
    xTaskCreate(taskGPS, "GPS_Task", 4096, NULL, 3, NULL);
    xTaskCreate(vMonitorTask, "Monitor", 2048, NULL, 1, NULL);
}

void loop() {}