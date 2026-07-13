#pragma once

/// @file Bluetooth.h
/// @authors Adnaan Juma
/// @version 1.0

#include "NimBLEServer.h"
#include "Telemetry.h"
#include <NimBLEDevice.h>

#include <cassert>
#include <format>

namespace mcu::bluetooth
{
class Manager
{
public:
  /// @brief Initializes the Bluetooth stack and manager.
  Manager();

  /// @brief Returns a telemetry callback instance for this Bluetooth manager.
  ///
  /// @return A @c Callback instance.
  mcu::telemetry::Manager::Callback getTelemetryCallback();

  /// @brief Returns the number of connected Bluetooth clients.
  /// @return The number of connected clients as a @c std::uint8_t.
  std::uint8_t getConnectedClients() const;

  /// @brief Cleans up the Bluetooth manager and releases resources.
  ~Manager();

private:
  // Init functions for each service
  void initFlightControlService(NimBLEServer& server);
  void initNavigationControlService(NimBLEServer& server);
  void initEnvironmentalControlService(NimBLEServer& server);
  void initActuatorControlService(NimBLEServer& server);

  // Updaters
  void updateFlightStatus(mcu::telemetry::FlightStatus status);
  void updateLinkStatus(bool connected, float rssi, float snr);
  void updateAttitude(float roll, float pitch, float yaw);
  void updateMapPosition(double latitude, double longitude);
  void updateAcceleration(float ax, float ay, float az, float gx, float gy,
                          float gz);
  void updateAltitude(float altitude);
  void updateVerticalVelocity(float verticalVelocity);
  void updatePressure(float pressure1, float pressure2);
  void updateTemperature(float temperature1, float temperature2);
  void updateAirbrakeExtension(float extension);

  NimBLECharacteristic *m_pFlightStatus, *m_pLinkStatus, *m_pAttitude,
      *m_pMapPosition, *m_pAcceleration, *m_pAltitude, *m_pVerticalVelocity,
      *m_pPressure, *m_pTemperature, *m_pAirbrakeExtension;
};

} // namespace mcu::bluetooth