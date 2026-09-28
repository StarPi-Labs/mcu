#pragma once

#include <Arduino.h>


struct BaroData {
	uint32_t timestamp;
	float temperature;
	float pressure;
	float altitude;
	int error_status;
};


bool barometer_setup(void);
void barometer_read(BaroData *sample1, BaroData *sample2);
