#ifndef BAROMETRO_H
#define BAROMETRO_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// Struttura che contiene tutti i dati del volo letti dal barometro
struct DatiBarometro {
    uint32_t timestamp;
    float temperatura;
    float pressione;
    float altitudine;
    int status_errore;
};

// Handle della coda: gli altri task useranno questa per leggere i dati
extern QueueHandle_t codaBarometro;

//funzioni varie
bool initBarometro();
void vTaskBarometro(void *pvParameters);

#endif