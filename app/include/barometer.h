#pragma once

#include <Arduino.h>


struct BaroData {
	uint32_t timestamp;
	float temperature; // degrees C
	float pressure;    // pascal
	float altitude;    // meters
	int error_status;
};


bool barometer_setup(void);
bool barometer_calibrate(unsigned int cal_time_ms);
void barometer_read(BaroData *sample1, BaroData *sample2);
bool is_baro_healthy_ground(float p1, float p2, float t1, float t2);
