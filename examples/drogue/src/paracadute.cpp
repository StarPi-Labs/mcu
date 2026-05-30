//MODIFICARE TUTTI I SERIAL PER LA COMUNICAZIONE E IL SALVATAGGIO SU SD

#include "paracadute.h"

//VALORI DA TARARE IN BASE A SIMULAZIONI E TEST
//ATTUALMENTE SOLTANTO STIMATI
#define THRESHOLD_MOTORE_OK 12.0
#define THRESHOLD_BURNOUT 2.0

StatoVolo statoAttuale = StatoVolo::PAD_IDLE;
float altitudineMassima = 0.0;
uint32_t tempoDecollo = 0;
uint32_t tempoApogeo = 0;
uint32_t tempoMain = 0;
bool motore_ok = false;


void initPara() {
    //Configurazione pin per i deploy
    pinMode(pinDrogueLS, OUTPUT);
    pinMode(pinMainLS, OUTPUT);
    digitalWrite(pinDrogueLS, LOW);
    digitalWrite(pinMainLS, LOW);
}


void valutaStato(DatiIMU imu, DatiBarometro baro) {
    switch (statoAttuale) {
        case StatoVolo::PAD_IDLE:
            //Rilevamento della fase di decollo basata su accelerazione verticale
            if (imu.acc_z > 3.0) { 
                statoAttuale = StatoVolo::BOOST;
                tempoDecollo = millis();
                Serial.println(">>> DECOLLO RILEVATO! <<<");
            }
            break;

        case StatoVolo::BOOST:
            //Verifica se il motore ha funzionato correttamente durante la fase di boost
            if (imu.acc_z >= THRESHOLD_MOTORE_OK) {
                motore_ok = true;
            }
            // Rilevamento del burnout del motore basato su una brusca diminuzione dell'accelerazione verticale
            if (imu.acc_z < THRESHOLD_BURNOUT && (millis() - tempoDecollo > 1000)) { 
                statoAttuale = StatoVolo::COAST;
                Serial.println(">>> BURNOUT MOTORE (COAST) <<<");
            }
            break;

        case StatoVolo::COAST:
            // Rilevamento apogeo (Velocità Z negativa). Il ritardo di 4s dal decollo evita aperture premature causate da turbolenze post-burnout
            if (imu.vel_z <= 0.0 && (millis() - tempoDecollo > 4000)) {
                
                tempoApogeo = millis();
                digitalWrite(pinDrogueLS, HIGH); 
                Serial.println(">>> APOGEO! <<<");

                //Gestione delle emergenze in caso di malfunzionamento del motore o apogeo troppo basso
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

            //Arrivo in quota per l'apertura del main
            if (baro.altitudine <= 450.0) {
                //Mandiamo il segnale per il rilascio del main
                digitalWrite(pinMainLS, HIGH);
                tempoMain = millis();
                Serial.println(">>> QUOTA RAGGIUNTA: MAIN DEPLOY<<<");
                statoAttuale = StatoVolo::MAIN_DEPLOY;
            }
            break;

        case StatoVolo::MAIN_DEPLOY:
            if (millis() - tempoMain > 2000) {
                //Lasciamo il segnale verso il main per 2 secondi, poi lo disattiviamo
                digitalWrite(pinMainLS, LOW); 
            }
            
            if (baro.altitudine < 10.0 || imu.acc_z > 20.0) { 
                statoAttuale = StatoVolo::TOUCHDOWN;
                Serial.println(">>> TOUCHDOWN <<<");
            }
            break;

        case StatoVolo::TOUCHDOWN:
            //Spegnamo tutto per sicurezza una volta atterrati
            digitalWrite(pinMainLS, LOW);
            digitalWrite(pinDrogueLS, LOW);
            break;
    }
}

void vTaskDatalogger(void *pvParameters) {
    PacchettoLog logRicevuto;
    for(;;) {
        if (xQueueReceive(codaLogger, &logRicevuto, portMAX_DELAY) == pdPASS) {  
            Serial.printf("T: %lu | Stato: %d | Alt: %.1f m | AccZ: %.1f g | VelZ: %.1f m/s\n", 
                          logRicevuto.timestamp, 
                          static_cast<int>(logRicevuto.stato),
                          logRicevuto.baro.altitudine, 
                          logRicevuto.imu.acc_z, 
                          logRicevuto.imu.vel_z);
        }
    }
}

//Main task della macchina a stati del paracadute, che si occupa di ricevere i dati dai sensori, 
//valutare lo stato di volo e inviare i pacchetti di log al datalogger
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