#include <unity.h>
#include <Arduino.h>
#include "tests.h"

void setUp() {
    
}

void tearDown() {

}

void setup() {
    delay(2000); // Attende che la connessione seriale sia stabilita
    runUnityTests();
}

void loop() {
    // Non facciamo nulla nel loop, i test vengono eseguiti una sola volta
}