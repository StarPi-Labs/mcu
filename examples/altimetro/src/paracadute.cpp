#include "paracadute.h"

// Definiamo fisicamente le variabili qui!
StatoVolo statoAttuale = StatoVolo::PAD_IDLE;
float altitudineMassima = 0.0;
uint32_t tempoDecollo = 0;
uint32_t tempoApogeo = 0;

extern QueueHandle_t codaIMU;
extern QueueHandle_t codaBarometro;
extern QueueHandle_t codaLogger;


void initPara() {
    pinMode(pinDrogueLS, OUTPUT);
    pinMode(pinMainLS, OUTPUT);
    digitalWrite(pinDrogueLS, LOW);
    digitalWrite(pinMainLS, LOW);
}

void valutaStato(DatiIMU imu, DatiBarometro baro) {
    switch (statoAttuale) {
        case StatoVolo::PAD_IDLE:
            if (imu.acc_z > 15.0) {
                statoAttuale = StatoVolo::BOOST;
                tempoDecollo = millis();
            }
            break;

        case StatoVolo::BOOST:
            if (imu.acc_z < 5.0) {
                statoAttuale = StatoVolo::COAST;
            }
            break;

        case StatoVolo::COAST:
            if (baro.altitudine > altitudineMassima) {
                altitudineMassima = baro.altitudine;
            }
            
            if (baro.altitudine < (altitudineMassima - 5.0) && (millis() - tempoDecollo > 5000)) {
                statoAttuale = StatoVolo::APOGEE_DEPLOY;
                tempoApogeo = millis();
                
                digitalWrite(pinDrogueLS, HIGH); 
            }
            break;

        case StatoVolo::APOGEE_DEPLOY:
            if (millis() - tempoApogeo > 2000) {
                digitalWrite(pinDrogueLS, LOW); 
            }

            if (baro.altitudine < 450.0) {
                statoAttuale = StatoVolo::MAIN_DEPLOY;
                digitalWrite(pinMainLS, HIGH);
            }
            break;

        case StatoVolo::MAIN_DEPLOY:
            
            if (baro.altitudine < 10.0 || imu.acc_z > 20.0) {
                statoAttuale = StatoVolo::TOUCHDOWN;
            }
            break;

        case StatoVolo::TOUCHDOWN:
            digitalWrite(pinMainLS, LOW);
            break;
    }
}


void vTaskPara(void *pvParameters) {
    initPara();
    const TickType_t frequenzaTask = pdMS_TO_TICKS(50);
    TickType_t ultimoRisveglio = xTaskGetTickCount();

    DatiIMU ultimoIMU;
    DatiBarometro ultimoBaro;
    PacchettoLog pacchetto;

    for (;;) {
        xQueueReceive(codaIMU, &ultimoIMU, 0);
        xQueueReceive(codaBarometro, &ultimoBaro, 0);

        valutaStato(ultimoIMU, ultimoBaro);

        pacchetto.timestamp = millis();
        pacchetto.baro = ultimoBaro;
        pacchetto.imu = ultimoIMU;
        pacchetto.stato = statoAttuale;
        
        xQueueSend(codaLogger, &pacchetto, 0); 

        vTaskDelayUntil(&ultimoRisveglio, frequenzaTask);
    }
}