#pragma once

#include <Arduino.h>
#include <LSM6DSO32Sensor.h>
#include <SPI.h>


#define IMU_ACCEL_FS LSM6DSO32_32g
#define IMU_GYRO_FS  LSM6DSO32_2000dps

// TODO: rename these to indicate that they are imu-specific
typedef struct {
	int32_t accelerometer[3];
	int32_t gyroscope[3];
	uint8_t timestamp[6];
} FIFO_Sample;

extern LSM6DSO32Sensor IMU;

void imu_setup();
/// @brief Reads a sample from the IMU.
/// When IMU_FIFO_ENABLE is set, all samples currently in the FIFO are averaged
/// into a single sample; otherwise the current sensor output is read directly.
/// @param sample Output sample. Accelerometer values are in mg (milli-g),
/// gyroscope values are in mdps (milli-degrees per second).
/// @return 0 on success, -1 if sample is NULL, the FIFO is empty, or the
/// accumulation overflowed.
int imu_get_sample(FIFO_Sample* sample);

float imu_calibrate(unsigned int cal_time_ms);
