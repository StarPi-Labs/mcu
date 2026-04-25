#include <Arduino.h>
#include "MS5611.h"

#include "barometer.h"
#include "logger.h"
#include "board.h"


#define BARO1_ADDRESS 0x77
#define BARO2_ADDRESS 0x76

extern TwoWire I2C1;
MS5611 baro1(BARO1_ADDRESS, &I2C1);
MS5611 baro2(BARO2_ADDRESS, &I2C1);


//https://github.com/RobTillaart/MS5611/blob/master/README.md
//https://github.com/RobTillaart/MS5611/blob/master/MS5611.cpp
//https://github.com/RobTillaart/MS5611/blob/master/MS5611.h

//--SETTING OSR--
//OSR_ULTRA_HIGH = 12,        // 10 ms
//OSR_HIGH       = 11,       //  5 ms
//OSR_STANDARD   = 10,        //  3 ms
//OSR_LOW        = 9,         //  2 ms
//OSR_ULTRA_LOW  = 8          //  1 ms
//impostato con OSR di 4096, se si vuole + veloce ma meno preciso si abbassa da 2^12 a 2^11 o 2^10


float ground_pressure_mbar_1 = 0.0; // memorizzo pressione misurata sulla rampa di lancio
float ground_pressure_mbar_2 = 0.0; // memorizzo pressione misurata sulla rampa di lancio


/*scaletta: 1. reset del barometro;
            2. qualche ciclo a vuoto per stabilizzarsi(Delta Sigma);
            3. Salva il valore di pressione sulla rampa di lancio;
            4. Ciclo infinito dove prende i valori e applica il filtro.

miglioramenti se funziona e si vuole upgradare:   1.si può far una media di 30-40 letture per percepire la pressione al suolo, perche ora se ne fa solo 1;
                                                  2.si aumentano il numero di elementi in coda;
                                                  3.si può aggiungere un ciclo di retry se restituisce false lo status.7

parametri di variazione: 1. Frequenza(ora a 20 hz);
                         2. OSR(ora a 12, quindi 4096 campioni e ci mette circa 9 ms);
                         3. alpha del filtro(ora a 0.15. Se si aumenta è più sensibile ai cambiamenti di quota, ma dato più tremolante);

*/

// Initialize the two barometers, returns false in case of error
bool barometer_setup(void)
{
	if (baro1.begin() == false) {
		ERR("Barometer 1 " TO_XSTR(BARO1_ADDRESS) " not found");
		return false; //se non funziona segnala l'errore e lascia svolgere le altre tasks
	}
	if (baro2.begin() == false) {
		ERR("Barometer 2 " TO_XSTR(BARO2_ADDRESS) " not found");
		return false; //se non funziona segnala l'errore e lascia svolgere le altre tasks
	}

	baro1.reset();
	LOG("Barometer 1 initialized");
	baro2.reset();
	LOG("Barometer 2 initialized");

	// legge un po' di volte per far stabilizzare il sensore
	// calibrate with the highest OSR to get the most accurate ground pressure
	for (int i = 0; i < 5; i++) {
		baro1.read(OSR_ULTRA_HIGH);
		baro2.read(OSR_ULTRA_HIGH);
		delay(50);
	}

	int status1 = baro1.read(OSR_ULTRA_HIGH);
	if (status1 == 0) {
		ground_pressure_mbar_1 = baro1.getPressure(); // autozero
		LOG("Barometer 1: Setting ground pressure to [mbar]", ground_pressure_mbar_1);
	} else {
		ERR("Barometer 1: Calibration failed with error code", status1);
	}

	int status2 = baro2.read(OSR_ULTRA_HIGH);
	if (status2 == 0) {
		ground_pressure_mbar_2 = baro2.getPressure(); // autozero
		LOG("Barometer 2: Setting ground pressure to [mbar]", ground_pressure_mbar_2);
	} else {
		ERR("Barometer 2: Calibration failed with error code", status2);
	}

	if (status1 != 0 || status2 != 0) {
		return false;
	}
	return true;
}


void barometer_read(BaroData *sample1, BaroData *sample2)
{
	if (sample1 == NULL || sample2 == NULL) return;

	sample1->error_status = baro1.read(BARO_OSR);
	sample2->error_status = baro2.read(BARO_OSR);

	// TODO: use the unified timestamp
	sample1->timestamp = sample2->timestamp = millis();

	// TODO: linear calibration
	if (sample1->error_status == 0) {
		sample1->temperature = baro1.getTemperature(); // temperatura in °C
		sample1->pressure = baro1.getPressure(); // pressione in mbar
		sample1->altitude = baro1.getAltitude(ground_pressure_mbar_1); // altitudine in metri
	}
	if (sample2->error_status == 0) {
		sample2->temperature = baro2.getTemperature(); // temperatura in °C
		sample2->pressure = baro2.getPressure(); // pressione in mbar
		sample2->altitude = baro2.getAltitude(ground_pressure_mbar_2); // altitudine in metri
	}
}
