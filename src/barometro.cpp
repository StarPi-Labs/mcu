#include "barometro.h"
#include <Wire.h>
#include "MS5611.h"

#define I2C_SDA_PIN 24 
#define I2C_SCL_PIN 22
#define BARO_ADDRESS 0x77 //0x77 per due dei 4 altimetri, gli altri due vanno a 0x76

MS5611 baro(BARO_ADDRESS);//istanza

//https://github.com/RobTillaart/MS5611/blob/master/README.md 
//https://github.com/RobTillaart/MS5611/blob/master/MS5611.cpp
//https://github.com/RobTillaart/MS5611/blob/master/MS5611.h

//--SETTING OSR--
//OSR_ULTRA_HIGH = 12,        // 10 ms
// OSR_HIGH       = 11,       //  5 ms
//OSR_STANDARD   = 10,        //  3 ms
//OSR_LOW        = 9,         //  2 ms
//OSR_ULTRA_LOW  = 8          //  1 ms

float pressioneAlSuolo_mbar = 0.0; //memorizzo pressione misurata sulla rampa di lancio
QueueHandle_t codaBarometro = NULL; //gestione della coda per freertos

//impostato con OSR di 4096, se si vuole + veloce ma meno preciso si abbassa da 2^12 a 2^11 o 2^10


/*scaletta: 1. reset del barometro;
            2. qualche ciclo a vuoto per stabilizzarsi(Delta Sigma);
            3. Salva il valore di pressione sulla rampa di lancio;
            4. Ciclo infinito dove prende i valori e applica il filtro.

miglioramenti se funziona e si vuole upgradare: 1.si può far una media di 30-40 letture per percepire la pressione al suolo, perche ora se ne fa solo 1;
                                                  2.si aumentano il numero di elementi in coda;
                                                  3.si può aggiungere un ciclo di retry se restituisce false lo status.7

parametri di variazione: 1. Frequenza(ora a 20 hz);
                         2. OSR(ora a 12, quindi 4096 campioni e ci mette circa 9 ms);
                         3. alpha del filtro(ora a 0.15. Se si aumenta è più sensibile ai cambiamenti di quota, ma dato più tremolante);
                         
*/                                              
bool initBarometro() {
    Serial.println("\n--- Inizializzazione Barometro MS5611 ---"); 
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    if (baro.begin() == false) {
        Serial.println("[ERRORE] Sensore MS5611 non trovato.");
        return false; //se non funziona segnala l'errore e lascia svolgere le altre tasks
    }

    baro.reset(); 
    Serial.println("Sensore inizializzato. Calibrazione Pressione al Suolo (AGL)...");
    
    //legge un po' di volte per far stabilizzare il sensore
    for(int i=0; i<5; i++) {
        baro.read(12); // OSR_ULTRA_HIGH
        vTaskDelay(pdMS_TO_TICKS(50));//per tot cicli(50) fai lavorare altre tasks (FREERTOS)
    }

    int status = baro.read(12); 
    if (status == 0) { //se è andato tutto bene status = 0
        pressioneAlSuolo_mbar = baro.getPressure(); //autozero
        Serial.printf("Pressione al suolo impostata a: %.2f mbar\n", pressioneAlSuolo_mbar);
        
        // coda per contenere fino a 10 letture se il ricevente è temporaneamente occupato. PROBABILMENTE DA CAMBIARE IL NUMERO DI LETTURE
        codaBarometro = xQueueCreate(10, sizeof(DatiBarometro));
        return true;
    } else {
        Serial.println("[ERRORE] Impossibile calibrare la pressione al suolo!");
        return false;
    }
}

//parte FREERTOS
void vTaskBarometro(void *pvParameters) {
    (void) pvParameters; 

    //frequenza del task (pe ora 20 Hz -> 50 ms di pausa tra un ciclo e un altro(lettura in 9 ms con OSR 4096))
    const TickType_t frequenzaTask = pdMS_TO_TICKS(50); 
    TickType_t ultimoRisveglio = xTaskGetTickCount();

    DatiBarometro datiAttuali;
    // Variabili per il filtro a media mobile
    float altitudine_filtrata = 0.0;
    const float alpha = 0.15; //da provare valori tra 0.1 e 0.3 
    bool prima_lettura = true;

    for (;;) {//ciclo infinito
        datiAttuali.timestamp = millis();
        datiAttuali.status_errore = baro.read(12); // OSR 4096 (max 9.04ms) 
        
        if (datiAttuali.status_errore == 0) {
            datiAttuali.temperatura = baro.getTemperature();
            datiAttuali.pressione = baro.getPressure();
            
            //prende il dato grezzo
        float altitudine_grezza = baro.getAltitude(pressioneAlSuolo_mbar);
        // applica il filtro
        if (prima_lettura) {
            altitudine_filtrata = altitudine_grezza; // Inizializza il filtro
            prima_lettura = false;
        } else {
            //formula filtro
            altitudine_filtrata = (alpha * altitudine_grezza) + ((1.0 - alpha) * altitudine_filtrata);
        }
            
        //Salva nella struct SOLO il dato pulito
        datiAttuali.altitudine = altitudine_filtrata;

            // Invia i dati alla coda per farli leggere ad altri task
            // Se la coda è piena, non bloccare (0 ticks di attesa)
            if (codaBarometro != NULL) {
                xQueueSend(codaBarometro, &datiAttuali, 0); //0 perchè se la coda è piena scarta il pacchetto di dati e prendi quello dopo, ma non bloccarti ad aspettare
            }
        } else {
            Serial.printf("[ERRORE BARO] Codice: %d\n", datiAttuali.status_errore);
        }

        // Aspetta il tempo esatto per mantenere i 20 Hz costanti
        vTaskDelayUntil(&ultimoRisveglio, frequenzaTask);
    }
}
