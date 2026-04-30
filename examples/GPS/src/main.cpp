#include <Arduino.h>
#include "gps_logger.h"

QueueHandle_t codaDatiGPS;

void vMonitorTask(void *pv) {
    ReportGPS r;
    for(;;) {
        if (xQueueReceive(codaDatiGPS, &r, portMAX_DELAY) == pdPASS) {
            Serial.printf("[GPS] Sat: %d | Lat: %.6f | Lon: %.6f | Alt: %.1f | PPS: %u\n", 
                          r.num_sat, r.lat, r.lon, r.alt_metri, r.pps_count);
        }
    }
}

void setup() {
    Serial.begin(115200);
    codaDatiGPS = xQueueCreate(10, sizeof(ReportGPS));

    xTaskCreate(taskGPS, "GPS_Task", 4096, NULL, 3, NULL);
    xTaskCreate(vMonitorTask, "Monitor", 2048, NULL, 1, NULL);
}

void loop() {}