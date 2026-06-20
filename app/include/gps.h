#pragma once

#include <Arduino.h>

typedef struct {
  uint64_t unix_time;
  double lat;
  double lon;
  float alt;
  float kmh;
  uint8_t num_sat;
} GPSData;

void gps_setup(void);
void gps_update(GPSData* data);
