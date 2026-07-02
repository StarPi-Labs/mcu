#pragma once

#include <Arduino.h>


#define LOG(s, ...)   Serial.printf(s "\n" __VA_OPT__(,) __VA_ARGS__)
#define WARN(s, ...)  Serial.printf("[WARN] " s "\n" __VA_OPT__(,) __VA_ARGS__)
#define ERR(s, ...)   Serial.printf("[ERR] " s "\n" __VA_OPT__(,) __VA_ARGS__)
