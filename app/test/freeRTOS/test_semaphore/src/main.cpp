#include <Arduino.h>
#include "tests.h"

void setup() {
    delay(2000); // Wait for serial connection
    runUnityTests();
}

void loop() {
}