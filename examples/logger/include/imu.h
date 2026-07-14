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

// rubato da CATS VEGA
struct calibration_data_t {
  FIFO_Sample gyro_calib;
  float angle;
  uint8_t axis;
};

extern LSM6DSO32Sensor IMU;
extern volatile bool in_free_fall;
extern calibration_data_t calib;

void imu_setup();
void update_sensitivity(const FIFO_Sample &sample);
void IRAM_ATTR check_free_fall();
int imu_get_sample(FIFO_Sample *sample);
void calibrate_gyro();
void correct_gyro(const calibration_data_t *calibration, FIFO_Sample *gyro_data);
bool compute_gyro_calibration(const FIFO_Sample *gyro_data, calibration_data_t *calibration);
