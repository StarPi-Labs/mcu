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

    SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    
    ReportGPS dati;

    for (;;) {
        while (SerialGPS.available() > 0) {
            parser.encode(SerialGPS.read());
        }

        if (parser.location.isUpdated()) {
            dati.ms_timestamp = millis();
            dati.lat = parser.location.lat();
            dati.lon = parser.location.lng();
            dati.alt_metri = parser.altitude.meters();
            dati.kmh = parser.speed.kmph();
            dati.num_sat = parser.satellites.value();
            dati.segnale_valido = parser.location.isValid();
            dati.pps_count = pps_counter;

            if (codaDatiGPS != NULL) {
                xQueueSend(codaDatiGPS, &dati, 0);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}