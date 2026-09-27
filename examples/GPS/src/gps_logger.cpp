#include <TinyGPS++.h>
#include "gps_logger.h"

HardwareSerial SerialGPS(1);
TinyGPSPlus parser;
extern QueueHandle_t codaDatiGPS;

volatile uint32_t pps_counter = 0;

void IRAM_ATTR pps_interrupt() {
    pps_counter++;
}

void taskGPS(void *parameter) {
    pinMode(GPS_RST_PIN, OUTPUT);
    digitalWrite(GPS_RST_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(GPS_RST_PIN, HIGH);

    pinMode(GPS_PPS_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(GPS_PPS_PIN), pps_interrupt, RISING);

    //Inizializziamo la seriale per il GPS
    SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    
    ReportGPS dati;

    for (;;) {
        while (SerialGPS.available() > 0) {
            parser.encode(SerialGPS.read());
        }

        //Fetchiamo i dati GPS se c'è un aggiornamento
        if (parser.location.isUpdated()) {
            dati.ms_timestamp = millis();
            dati.num_sat = parser.satellites.value();
            dati.pps_count = pps_counter;

            //Aggiorniamo latitudine e longitudine se il segnale è valido, altrimenti poniamo a zero fino al prossimo aggiornamento
            //Consideriamo utilizzabile un segnale di posizione se abbiamo almeno 3 satelliti in vista
            if (parser.location.isValid() && dati.num_sat >= 3) {
                dati.lat = parser.location.lat();
                dati.lon = parser.location.lng();
                dati.segnale_valido = true;
            } else {
                dati.lat = 0.0;
                dati.lon = 0.0;
                dati.segnale_valido = false;
            }

            //Aggiorniamo se il segnale è valido, altrimenti poniamo a zero fino al prossimo aggiornamento
            //Consideriamo utilizzabile un segnale di altitudine se abbiamo almeno 4 satelliti in vista
            if (parser.altitude.isValid() && dati.num_sat >= 4) {
                dati.alt_metri = parser.altitude.meters();
            } else {
                dati.alt_metri = 0.0;
            }

            //Idem per la velocità 
            if (parser.speed.isValid() && dati.num_sat >= 4) {
                dati.kmh = parser.speed.kmph();
            } else {
                dati.kmh = 0.0;
            }

            //Spediamo i dati alla coda se è stata creata correttamente
            if (codaDatiGPS != NULL) {
                xQueueSend(codaDatiGPS, &dati, 0);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void vMonitorTask(void *pv) {
    ReportGPS r;
    //Sintetizziamo i dati GPS ricevuti dalla coda e li stampiamo sul seriale
    for(;;) {
        if (xQueueReceive(codaDatiGPS, &r, portMAX_DELAY) == pdPASS) {
            Serial.printf("[GPS] Sat: %d | Lat: %.6f | Lon: %.6f | Alt: %.1f | PPS: %u\n", 
                          r.num_sat, r.lat, r.lon, r.alt_metri, r.pps_count);
        }
    }
}