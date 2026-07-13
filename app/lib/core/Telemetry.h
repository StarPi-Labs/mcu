#pragma once

/// @file Telemetry.h
/// @brief Telemetry system for collecting and transmitting data.
/// @author Adnaan Juma
/// @version 1.0

#include <concepts>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <utility>

namespace mcu::telemetry
{
/// @brief Represents the different states of the rocket for telemetry
/// purposes.
enum class FlightStatus : std::uint8_t {
  IDLE,
  ARMED,
  BOOST,
  COAST,
  AIRBRAKE,
  DEPLOY,
  DESCENT
};

/// @brief A @b synchronous telemetry manager that handles the collection and
/// transmission of telemetry data. It can manage multiple callback objects that
/// respond to telemetry updates.
class Manager
{
public:
  /// @brief A callback interface for telemetry updates.
  struct Callback {
    void* context; //< User-defined context pointer for callback functions.
    void (*onFlightStatusUpdate)(FlightStatus status, void* context);
    void (*onLinkStatusUpdate)(bool connected, float rssi, float snr,
                               void* context);
    void (*onAttitudeUpdate)(float roll, float pitch, float yaw, void* context);
    void (*onMapPositionUpdate)(double latitude, double longitude,
                                void* context);
    void (*onAccelerationUpdate)(float ax, float ay, float az, float gx,
                                 float gy, float gz, void* context);
    void (*onAltitudeUpdate)(float altitude, void* context);
    void (*onVerticalVelocityUpdate)(float verticalVelocity, void* context);
    void (*onPressureUpdate)(float pressure1, float pressure2, void* context);
    void (*onTemperatureUpdate)(float temperature1, float temperature2,
                                void* context);
    void (*onAirbrakeExtensionUpdate)(float extension, void* context);
  };

  /// @brief Default constructor initializes the telemetry manager with no
  /// callbacks.
  Manager();

  /// @brief Initializes the telemetry manager with a list of callbacks.
  ///
  /// @param callbacks A list of @c Callback objects to be managed by the
  /// telemetry manager.
  Manager(std::initializer_list<Callback> callbacks);

  Manager(const Manager&) = delete;
  Manager& operator=(const Manager&) = delete;
  Manager(Manager&&) = delete;
  Manager& operator=(Manager&&) = delete;

  /// @brief Pushes a new callback to the telemetry manager.
  ///
  /// @param callback A @c Callback object to be added to the telemetry manager.
  void pushCallback(const Callback& callback);

  /// @brief Emplaces a new callback to the telemetry manager.
  ///
  /// @tparam Args The types of the arguments to construct the callback.
  /// @param args The arguments to construct the callback.
  /// @pre The arguments must be constructible into a @c Callback object.
  template <typename... Args>
    requires(std::constructible_from<Callback, Args...>)
  void emplaceCallback(Args&&... args)
  {
    assert(isValidCallback(Callback{std::forward<Args>(args)...}) &&
           "Invalid callback: all function pointers must be non-nullptr");
    m_callbacks.emplace_back(std::forward<Args>(args)...);
  }

  /// @brief Removes the last callback from the telemetry manager.
  ///
  /// @pre There must be at least one callback in the manager.
  void popBackCallback();

  /// @brief Updates the current state of the rocket for telemetry
  /// transmission.
  ///
  /// @param state A @c State enum value representing the current state of the
  /// rocket.
  void updateFlightStatus(FlightStatus status);

  /// @brief Updates the link status for telemetry transmission.
  ///
  /// @param connected Whether the LoRa link is currently established.
  /// @param rssi Received Signal Strength Indicator in dBm.
  /// @param snr Signal-to-Noise Ratio in dB.
  void updateLinkStatus(bool connected, float rssi, float snr);

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
  void updateMapPosition(double latitude, double longitude);

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
  /// @brief Checks if a given callback is valid: all fptrs are non-nullptr
  ///
  /// @param callback The callback to validate.
  /// @return True if the callback is valid, false otherwise.
  static bool isValidCallback(const Callback& callback);

  std::vector<Callback>
      m_callbacks; //< List of registered callbacks for telemetry updates.
};
} // namespace mcu::telemetry