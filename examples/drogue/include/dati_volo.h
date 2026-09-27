#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>


struct DatiBarometro {
    uint32_t timestamp;  // [ms]
    float temperatura;   // [°C]
    float pressione;     // [mbar]
    float altitudine;    // [m]
    int status_errore;   // 0 = OK
};

struct DatiIMU {
    uint32_t timestamp;  // [ms]
    
    // Accelerometro [G]
    float acc_x;
    float acc_y;
    float acc_z;       
    
    // Giroscopio [dps]
    float gyro_x;
    float gyro_y;
    float gyro_z;
    
    float vel_z;
    
    int status_errore;   // 0 = OK
};


enum class StatoVolo {
    PAD_IDLE,       // Fermo in rampa
    BOOST,          // Motore acceso (Acc_z > 3G)
    COAST,          // Motore spento, in salita per inerzia
    APOGEE_DEPLOY,  // Apogeo raggiunto, drogue espulso
    MAIN_DEPLOY,    // Quota main raggiunta, main espulso
    TOUCHDOWN       // Atterrato
};

struct PacchettoLog {
    uint32_t timestamp;  // [ms]
    DatiBarometro baro;
    DatiIMU imu;
    StatoVolo stato;
};


extern QueueHandle_t codaIMU;
extern QueueHandle_t codaBarometro;
extern QueueHandle_t codaLogger;