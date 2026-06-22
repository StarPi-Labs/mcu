#pragma once

/// @file Telemetry.h
/// @brief Telemetry system for collecting and transmitting data.
/// @author Adnaan Juma
/// @version 1.0

// I would need:
// * Attitude (roll, pitch, yaw)
// * Accelerations and angular moments
// * Pressure from both barometers
// * State (idle, armed, boost, coast, airbrake, deploy, descent)
// * Altitude
// * Airbrake extension percentage
// * Two temperatures
// * LoRa link status (RSSI I think)
// * Position
// * Vertical velocity
// It would be nice to see the position on a map and velocity/attitude on a 3D
// model

#include <concepts>
#include <tuple>

namespace mcu::telemetry
{
/// @brief Represents the different states of the rocket for telemetry
/// purposes.
enum class FlightStatus {
  IDLE,
  ARMED,
  BOOST,
  COAST,
  AIRBRAKE,
  DEPLOY,
  DESCENT
};

/// @brief Represents the status of the LoRa link for telemetry purposes.
struct LoRaLinkStatus {
  bool connected; // Whether the LoRa link is currently established
  float rssi;     // Received Signal Strength Indicator in dBm
  float snr;      // Signal-to-Noise Ratio in dB
};

template <typename T>
concept ManagerCallback = requires(T t) {
  requires requires(FlightStatus status) {
    { t.onFlightStatusUpdate(status) } -> std::same_as<void>;
  } && requires(const LoRaLinkStatus& status) {
    { t.onLinkStatusUpdate(status) } -> std::same_as<void>;
  } && requires(float roll, float pitch, float yaw) {
    { t.onAttitudeUpdate(roll, pitch, yaw) } -> std::same_as<void>;
  } && requires(float latitude, float longitude) {
    { t.onMapPositionUpdate(latitude, longitude) } -> std::same_as<void>;
  } && requires(float ax, float ay, float az, float gx, float gy, float gz) {
    { t.onAccelerationUpdate(ax, ay, az, gx, gy, gz) } -> std::same_as<void>;
  } && requires(float altitude) {
    { t.onAltitudeUpdate(altitude) } -> std::same_as<void>;
  } && requires(float verticalVelocity) {
    { t.onVerticalVelocityUpdate(verticalVelocity) } -> std::same_as<void>;
  } && requires(float pressure1, float pressure2) {
    { t.onPressureUpdate(pressure1, pressure2) } -> std::same_as<void>;
  } && requires(float temperature1, float temperature2) {
    { t.onTemperatureUpdate(temperature1, temperature2) } -> std::same_as<void>;
  } && requires(float extension) {
    { t.onAirbrakeExtensionUpdate(extension) } -> std::same_as<void>;
  };
};

template <typename... Callbacks>
  requires(ManagerCallback<Callbacks> && ...)
class Manager
{
public:
  /// @brief Updates the current state of the rocket for telemetry
  /// transmission.
  ///
  /// @param state A @c State enum value representing the current state of the
  /// rocket.
  void updateFlightStatus(FlightStatus status);

  /// @brief Updates the link status for telemetry transmission.
  ///
  /// @param status A @c LoRaLinkStatus struct representing the current link
  /// status
  void updateLinkStatus(const LoRaLinkStatus& status);

  /// @brief Updates the rotation axes (roll, pitch, yaw) for telemetry
  /// transmission.
  ///
  /// @param roll Roll angle in degrees.
  /// @param pitch Pitch angle in degrees.
  /// @param yaw Yaw angle in degrees.
  void updateAttitude(float roll, float pitch, float yaw);

  /// @brief Updates the position (latitude, longitude) for telemetry
  /// transmission.
  ///
  /// @param latitude Latitude in degrees.
  /// @param longitude Longitude in degrees.
  void updateMapPosition(float latitude, float longitude);

  /// @brief Updates the acceleration values for telemetry transmission.
  ///
  /// @param ax Acceleration along the x-axis in g.
  /// @param ay Acceleration along the y-axis in g.
  /// @param az Acceleration along the z-axis in g.
  /// @param gx Angular velocity around the x-axis in degrees per second.
  /// @param gy Angular velocity around the y-axis in degrees per second.
  /// @param gz Angular velocity around the z-axis in degrees per second.
  void updateAcceleration(float ax, float ay, float az, float gx, float gy,
                          float gz);

  /// @brief Updates the altitude for telemetry transmission.
  ///
  /// @param altitude Altitude in meters.
  void updateAltitude(float altitude);

  /// @brief Updates the vertical velocity for telemetry transmission.
  ///
  /// @param verticalVelocity Vertical velocity in meters per second.
  void updateVerticalVelocity(float verticalVelocity);

  /// @brief Updates the pressure readings from two barometers for telemetry
  /// transmission.
  ///
  /// @param pressure1 Pressure reading from the first barometer in mBar.
  /// @param pressure2 Pressure reading from the second barometer in mBar.
  void updatePressure(float pressure1, float pressure2);

  /// @brief Updates the temperature readings for telemetry transmission.
  ///
  /// @param temperature1 Temperature reading from the first sensor in degrees.
  /// @param temperature2 Temperature reading from the second sensor in degrees.
  void updateTemperature(float temperature1, float temperature2);

  /// @brief Updates the airbrake extension
  ///
  /// @param extension Airbrake extension percentage [0.0, 1.0]
  void updateAirbrakeExtension(float extension);

private:
  std::tuple<Callbacks...> m_callbacks;
};
} // namespace mcu::telemetry