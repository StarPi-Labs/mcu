#pragma once

#include <Arduino.h>
#include <LSM6DSO32Sensor.h>
#include <SPI.h>


// TODO: rename these to indicate that they are imu-specific
typedef struct {
	int32_t accelerometer[3];
	int32_t gyroscope[3];
	uint8_t timestamp[6];
} FIFO_Sample;


extern LSM6DSO32Sensor IMU;


void imu_setup();
int imu_get_sample(FIFO_Sample *sample);
