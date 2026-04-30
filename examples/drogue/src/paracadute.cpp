#include "paracadute.h"

StatoVolo statoAttuale = StatoVolo::PAD_IDLE;
float altitudineMassima = 0.0;
uint32_t tempoDecollo = 0;
uint32_t tempoApogeo = 0;
uint32_t tempoMain = 0;
bool motore_ok = false;

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
            if (imu.acc_z > 3.0) { 
                statoAttuale = StatoVolo::BOOST;
                tempoDecollo = millis();
                Serial.println(">>> DECOLLO RILEVATO! <<<");
            }
            break;

        case StatoVolo::BOOST:
            if (imu.acc_z >= 18.0) {
                motore_ok = true;
            }
            
            if (imu.acc_z < 2.0 && (millis() - tempoDecollo > 1000)) { 
                statoAttuale = StatoVolo::COAST;
                Serial.println(">>> BURNOUT MOTORE (COAST) <<<");
            }
            break;

        case StatoVolo::COAST:
            // Rilevamento Apogeo
            if (imu.vel_z <= 0.0 && (millis() - tempoDecollo > 4000)) {
                
                tempoApogeo = millis();
                digitalWrite(pinDrogueLS, HIGH); 
                Serial.println(">>> APOGEO! <<<");

                // Gestione Emergenze
                if (motore_ok == true || baro.altitudine > 450.0) {
                    statoAttuale = StatoVolo::APOGEE_DEPLOY;
                } else {
                    Serial.println("!!! EMERGENZA UNDERBURN !!!");
                    digitalWrite(pinMainLS, HIGH);
                    tempoMain = millis();
                    statoAttuale = StatoVolo::MAIN_DEPLOY; 
                }
            }
            break;

        case StatoVolo::APOGEE_DEPLOY:
            if (millis() - tempoApogeo > 2000) {
                digitalWrite(pinDrogueLS, LOW); 
            }

            // Attesa quota Main
            if (baro.altitudine <= 450.0) {
                digitalWrite(pinMainLS, HIGH);
                tempoMain = millis();
                Serial.println(">>> QUOTA RAGGIUNTA: MAIN DEPLOY<<<");
                statoAttuale = StatoVolo::MAIN_DEPLOY;
            }
            break;

        case StatoVolo::MAIN_DEPLOY:
            if (millis() - tempoMain > 2000) {
                digitalWrite(pinMainLS, LOW); 
            }
            
            if (baro.altitudine < 10.0 || imu.acc_z > 20.0) { 
                statoAttuale = StatoVolo::TOUCHDOWN;
                Serial.println(">>> TOUCHDOWN <<<");
            }
            break;

        case StatoVolo::TOUCHDOWN:
            digitalWrite(pinMainLS, LOW);
            digitalWrite(pinDrogueLS, LOW);
            break;
    }
}

void vTaskPara(void *pvParameters) {
    initPara();
    const TickType_t frequenzaTask = pdMS_TO_TICKS(50);
    TickType_t ultimoRisveglio = xTaskGetTickCount();

    DatiIMU ultimoIMU = {0}; 
    DatiBarometro ultimoBaro = {0};
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