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


float ground_pressure_pa_1 = 0.0; // memorizzo pressione misurata sulla rampa di lancio
float ground_pressure_pa_2 = 0.0; // memorizzo pressione misurata sulla rampa di lancio
float ground_temperature_k = 0.0;
float ground_reference_altitude_m = 0.0f; // altitudine alla calibrazione calcolata con atmosfera standard dal livello del mare


// NOTE: cats vega as reference -> sea level temp and pressure are 15°C and 101250 Pa, respectively
//       as such baro calibration is useless, this should be changed if we don't use cats vega as reference
// https://www.mide.com/air-pressure-at-altitude-calculator
float compute_altitude(float air_pressure, float ground_pressure)
{
	(void)ground_pressure;
	constexpr float ground_reference_temperature_k = 15.0f + 273.15f; // standard temperature at sea level
	constexpr float ground_reference_pressure_pa = 101250.0f;

	// FIXME: Lb and M change with weather conditions
	const float Lb = -0.0065; // standard temperature lapse rate [K/m]
	const float M = 0.0289644; // Molar mass of air [kg/mol]
	const float R = 8.31432;  // Universal gas constant [N*m/mol*K]
	const float g0 = 9.80665; // Gravitational constant [m/s^2]

	const float exponent = -(R*Lb)/(g0*M);
	float temperature_rise = ground_reference_temperature_k / Lb;
	float pressure_ratio = air_pressure / ground_reference_pressure_pa;

	float altitude =  temperature_rise * (powf(pressure_ratio, exponent) - 1) - ground_reference_altitude_m;

	return altitude;
}


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
		log(S_BARO, T_SYSLOG, "[ERR] Barometer 1 " TO_XSTR(BARO1_ADDRESS) " not found");
		return false; //se non funziona segnala l'errore e lascia svolgere le altre tasks
	}
	if (baro2.begin() == false) {
		log(S_BARO, T_SYSLOG, "[ERR] Barometer 2 " TO_XSTR(BARO2_ADDRESS) " not found");
		return false; //se non funziona segnala l'errore e lascia svolgere le altre tasks
	}

	baro1.reset();
	log(S_BARO, T_SYSLOG, "Barometer 1 initialized");
	baro2.reset();
	log(S_BARO, T_SYSLOG, "Barometer 2 initialized");

	// legge un po' di volte per far stabilizzare il sensore
	// calibrate with the highest OSR to get the most accurate ground pressure
	uint64_t start_time = millis();
	const uint64_t baro_settle_time = 2000;
	while (millis() - start_time < baro_settle_time) {
		baro1.read(OSR_ULTRA_HIGH);
		baro2.read(OSR_ULTRA_HIGH);
		delay(10); // 10 ms delay to avoid flooding the barometer with requests
	}

	return true;
}


bool barometer_calibrate(unsigned int cal_time_ms)
{
	float p1_median = 0, p2_median = 0, t1_median = 0, t2_median = 0;
	uint32_t samples = 0;
	uint64_t start_time = millis();
	while (millis() - start_time < cal_time_ms) {
		int status1 = baro1.read(OSR_ULTRA_HIGH);
		int status2 = baro2.read(OSR_ULTRA_HIGH);

		// FIXME: log the error code
		if (status1 != 0) {
			log(S_BARO, T_SYSLOG, "[ERR] Barometer 1: Calibration failed");
			return false;
		}
		if (status2 != 0) {
			log(S_BARO, T_SYSLOG, "[ERR] Barometer 2: Calibration failed");
			return false;
		}

		p1_median += baro1.getPressurePascal();
		p2_median += baro2.getPressurePascal();
		t1_median += baro1.getTemperature();
		t2_median += baro2.getTemperature();
		samples++;

		delay(10); // 10 ms delay to avoid flooding the barometer with requests
	}

	p1_median /= samples;
	p2_median /= samples;
	t1_median /= samples;
	t2_median /= samples;

	ground_pressure_pa_1 = p1_median;
	ground_pressure_pa_2 = p2_median;

	float m = (ground_pressure_pa_1+ground_pressure_pa_2)/2.0f;
	ground_reference_altitude_m = compute_altitude(m, m);

	log(S_BARO, T_PRESSURE, ground_pressure_pa_1, ground_pressure_pa_2);
	log(S_BARO, T_TEMPERATURE, t1_median, t2_median);
	ground_temperature_k = (t1_median+t2_median)/2.0 + 273.15;

	return true;
}


void barometer_set_ground_temp_c(float temp_c)
{
	ground_temperature_k = temp_c + 273.15;
}


void barometer_read(BaroData *sample1, BaroData *sample2)
{
	if (sample1 == NULL || sample2 == NULL) return;

	sample1->error_status = baro1.read(BARO_OSR);
	sample2->error_status = baro2.read(BARO_OSR);

	// TODO: use the unified timestamp
	sample1->timestamp = sample2->timestamp = millis();

	if (sample1->error_status == 0) {
		sample1->temperature = baro1.getTemperature(); // temperatura in °C
		sample1->pressure = baro1.getPressurePascal(); // pressione in Pa
		sample1->altitude = compute_altitude(sample1->pressure, ground_pressure_pa_1); // altitudine in metri
	}
	if (sample2->error_status == 0) {
		sample2->temperature = baro2.getTemperature(); // temperatura in °C
		sample2->pressure = baro2.getPressurePascal(); // pressione in Pa
		sample2->altitude = compute_altitude(sample2->pressure, ground_pressure_pa_2); // altitudine in metri
	}
}


bool is_baro_healthy_ground(float p1, float p2, float t1, float t2) {
	
	static int count = 0;
	static constexpr int max_count = 10; 

	bool diff_ok = abs(p1 - p2) < 600 && abs(t1 - t2) < 40.0f; //baro pressure delta approx. 50m


	float pressure = 0.5f * (p1 + p2);
	float temperature = 0.5f * (t1 + t2);

	bool temp_ok = temperature < 70.0f && temperature > 10.0f;
	bool pressure_ok = pressure < 104000.0f && pressure > 94000.0f;


	static bool variance_ok = true;

	static constexpr float variance_thr = 100.0f * 2.4f * 2.4f; 
	static constexpr int samples = 10;
	static float press_samples[samples];
	static int sample_index = 0;

	press_samples[sample_index] = pressure;

	sample_index = (sample_index + 1) % samples;

	if (sample_index == 0){
		float press_mean = 0;

		for (int i = 0; i < samples; i++){
			press_mean += press_samples[i];
		}
		press_mean /= samples;

		float press_variance = 0;
		for (int i = 0; i < samples; i++){
			press_variance += (press_samples[i] - press_mean) * (press_samples[i] - press_mean);
		}
		press_variance /= samples;

		variance_ok = press_variance < variance_thr && press_variance > std::numeric_limits<float>::epsilon();
	} 

	if (!diff_ok || !temp_ok || !pressure_ok || !variance_ok) count++;
	else count = 0;

	return count < max_count;
}
