#include "HardwareSerial.h"
#include <unity.h>

#include <Arduino.h>
#include <Bluetooth.h>
#include <Telemetry.h>

void setUp(void) {}

void tearDown(void) {}

void test_flight_status_update()
{
  mcu::bluetooth::Manager bluetoothManager;
  mcu::telemetry::Manager<mcu::bluetooth::Manager::TelemetryCallback>
      telemetryManager(bluetoothManager.getTelemetryCallback());

  Serial.print("Waiting for Bluetooth client to connect...");
  while (!bluetoothManager.getConnectedClients()) {
    delay(1000);
    Serial.print(".");
    Serial.flush();
  }

  Serial.println("\nBluetooth client connected. Waiting 10 seconds...");
  delay(10000);

  Serial.println("Sending flight status updates.");

  constexpr std::array FLIGHT_STATUS_NAMES = {
      "IDLE", "ARMED", "BOOST", "COAST", "AIRBRAKE", "DEPLOY", "DESCENT"};

  for (uint8_t i = 0; i < FLIGHT_STATUS_NAMES.size() - 1; ++i) {
    Serial.printf("Current: %s, next: %s\n", FLIGHT_STATUS_NAMES[i],
                  FLIGHT_STATUS_NAMES[i + 1]);
    delay(3000);
    telemetryManager.updateFlightStatus(
        static_cast<mcu::telemetry::FlightStatus>(i + 1));
  }

  Serial.print("Waiting for Bluetooth client to disconnect...");

  while (bluetoothManager.getConnectedClients()) {
    delay(1000);
    Serial.print(".");
    Serial.flush();
  }

  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_flight_status_update);
  UNITY_END();
}

void loop()
{
  // Do nothing
}