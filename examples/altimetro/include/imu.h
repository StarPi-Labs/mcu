#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

struct DatiIMU {
    uint32_t timestamp;
    float acc_x;
    float acc_y;
    float acc_z;
    float vel_x;
    float vel_y;
    float vel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    int status_errore;
};

extern QueueHandle_t codaIMU;

bool initIMU();
void vTaskIMU(void *pvParameters);

#endif