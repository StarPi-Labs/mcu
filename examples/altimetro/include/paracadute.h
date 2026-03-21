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

class LogicaParacadute {
private:
    StatoVolo statoAttuale;
    uint32_t tempoDecollo;
    const int pinDrogue = 4;
    const int pinMain = 2;

public:
    LogicaParacadute(); 
    void init();
    void valutaStato(DatiBarometro baro, DatiIMU imu);
    StatoVolo getStatoAttuale();
};