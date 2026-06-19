#pragma once
#include <Arduino.h>
#include "barometro.h"
#include "imu.h"

enum class StatoVolo {
    PAD_IDLE,
    BOOST,
    COAST,
    APOGEE_DEPLOY,
    MAIN_DEPLOY,
    TOUCHDOWN
};

struct PacchettoLog {
    uint32_t timestamp;
    DatiBarometro baro;
    DatiIMU imu;
    StatoVolo stato;
};

extern QueueHandle_t codaLogger;