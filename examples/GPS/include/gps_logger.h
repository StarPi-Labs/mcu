#pragma once
#include <Arduino.h>

#define GPS_RX_PIN 18  // Collegato a GPS_TX
#define GPS_TX_PIN 17  // Collegato a GPS_RX
#define GPS_RST_PIN 9  // Collegato a GPS_RST
#define GPS_PPS_PIN 8  // Collegato a GPS_PPS

struct ReportGPS {
    uint32_t ms_timestamp;
    double lat;
    double lon;
    float alt_metri;
    float kmh;
    uint8_t num_sat;
    bool segnale_valido;
    uint32_t pps_count;
};

void taskGPS(void *parameter);
void vMonitorTask(void *pv);