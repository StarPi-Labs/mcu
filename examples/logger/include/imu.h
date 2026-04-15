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

typedef struct {
  FIFO_Sample fifo_batch[148]; //3072 bytes / () = 114
  int index = 0;
} FIFO_BATCH;


extern LSM6DSO32Sensor IMU;


void imu_setup();
void imu_restart();
int imu_get_sample(FIFO_Sample *sample);
