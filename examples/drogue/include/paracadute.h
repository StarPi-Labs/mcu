#pragma once
#include "dati_volo.h"

constexpr int pinDrogueLS = 38;
constexpr int pinDrogueHS = 39;
constexpr int pinMainLS = 40;
constexpr int pinMainHS = 41;

extern StatoVolo statoAttuale;
extern float altitudineMassima;
extern uint32_t tempoDecollo;
extern uint32_t tempoApogeo;
extern uint32_t tempoMain;
extern bool motore_ok;

void initPara();
void valutaStato(DatiIMU imu, DatiBarometro baro);
void vTaskPara(void *pvParameters);