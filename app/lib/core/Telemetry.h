#pragma once

/// @file Telemetry.h
/// @brief Telemetry system for collecting and transmitting data.
/// @author Adnaan Juma
/// @version 1.0

#include <concepts>
#include <cstdint>
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

/// @brief A concept that defines the required interface for a telemetry
/// callback. Any class that wants to receive telemetry updates must implement
/// this interface.
template <typename T>
concept ManagerCallback = requires(T t) {
  requires requires(FlightStatus status) {
    { t.onFlightStatusUpdate(status) } -> std::same_as<void>;
  } && requires(bool connected, float rssi, float snr) {
    { t.onLinkStatusUpdate(connected, rssi, snr) } -> std::same_as<void>;
  } && requires(float roll, float pitch, float yaw) {
    { t.onAttitudeUpdate(roll, pitch, yaw) } -> std::same_as<void>;
  } && requires(double latitude, double longitude) {
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

/// @brief A @b synchronous telemetry manager that handles the collection and
/// transmission of telemetry data. It can manage multiple callback objects that
/// respond to telemetry updates.
///
/// @tparam Callbacks A variadic template parameter pack for the callback types.
/// each type must satisfy the @c ManagerCallback concept.
/// @pre At least one callback type must be provided.
template <typename... Callbacks>
  requires(sizeof...(Callbacks) >= 1 && (ManagerCallback<Callbacks> && ...))
class Manager
{
public:
  /// @brief Default constructor: calls the default constructor of each
  /// callback.
  ///
  /// @see https://en.cppreference.com/cpp/utility/tuple/tuple (1) for details
  /// on the requirements for this constructor to be explicit/available.
  constexpr explicit(!requires { [](std::tuple<Callbacks...>) {}({}); })
      Manager()
    requires requires { std::tuple<Callbacks...>(); }
      : m_callbacks()
  {
  }

  /// @brief Direct constructor: initializes each callback with the
  /// correspinding argument.
  ///
  /// @param callbacks The callback objects to be used for telemetry updates.
  ///
  /// @see https://en.cppreference.com/w/cpp/utility/tuple/tuple (2) for
  /// details on the requirements for this constructor to be explicit/available.
  constexpr explicit(!requires {
    [](std::tuple<Callbacks...>) {}({std::declval<Callbacks>()...});
  }) Manager(const Callbacks&... callbacks)
    requires requires { std::tuple<Callbacks...>(callbacks...); }
      : m_callbacks(callbacks...)
  {
  }

  /// @brief Converting constructor: initializes each callback with the
  /// corresponding value in @c std::forward<Args>(args)
  ///
  /// @tparam Args The types of the arguments to be forwarded to the callbacks.
  /// @param args The arguments to be forwarded to the callbacks.
  ///
  /// @see https://en.cppreference.com/w/cpp/utility/tuple/tuple (3) for details
  /// on the requirements for this constructor to be explicit/available.
  template <typename... Args>
  constexpr explicit(!requires {
    [](std::tuple<Callbacks...>) {}({std::declval<Args>()...});
  }) Manager(Args&&... args)
    requires requires { std::tuple<Callbacks...>(std::forward<Args>(args)...); }
      : m_callbacks(std::forward<Args>(args)...)
  {
  }

  // Deleted copy and move constructors to prevent copying or moving the manager
  constexpr Manager(const Manager&) = delete;
  constexpr Manager& operator=(const Manager&) = delete;
  constexpr Manager(Manager&&) = delete;
  constexpr Manager& operator=(Manager&&) = delete;

  /// @brief Updates the current state of the rocket for telemetry
  /// transmission.
  ///
  /// @param state A @c State enum value representing the current state of the
  /// rocket.
  constexpr void updateFlightStatus(FlightStatus status)
  {
    std::apply(
        [=](auto&... callback) {
          (callback.onFlightStatusUpdate(status), ...);
        },
        m_callbacks);
  }

  /// @brief Updates the link status for telemetry transmission.
  ///
  /// @param connected Whether the LoRa link is currently established.
  /// @param rssi Received Signal Strength Indicator in dBm.
  /// @param snr Signal-to-Noise Ratio in dB.
  constexpr void updateLinkStatus(bool connected, float rssi, float snr)
  {
    std::apply(
        [&](auto&... callback) {
          (callback.onLinkStatusUpdate(connected, rssi, snr), ...);
        },
        m_callbacks);
  }

  /// @brief Updates the rotation axes (roll, pitch, yaw) for telemetry
  /// transmission.
  ///
  /// @param roll Roll angle in degrees.
  /// @param pitch Pitch angle in degrees.
  /// @param yaw Yaw angle in degrees.
  constexpr void updateAttitude(float roll, float pitch, float yaw)
  {
    std::apply(
        [=](auto&... callback) {
          (callback.onAttitudeUpdate(roll, pitch, yaw), ...);
        },
        m_callbacks);
  }

  /// @brief Updates the position (latitude, longitude) for telemetry
  /// transmission.
  ///
  /// @param latitude Latitude in degrees.
  /// @param longitude Longitude in degrees.
  constexpr void updateMapPosition(double latitude, double longitude)
  {
    std::apply(
        [=](auto&... callback) {
          (callback.onMapPositionUpdate(latitude, longitude), ...);
        },
        m_callbacks);
  }

  /// @brief Updates the acceleration values for telemetry transmission.
  ///
  /// @param ax Acceleration along the x-axis in g.
  /// @param ay Acceleration along the y-axis in g.
  /// @param az Acceleration along the z-axis in g.
  /// @param gx Angular velocity around the x-axis in degrees per second.
  /// @param gy Angular velocity around the y-axis in degrees per second.
  /// @param gz Angular velocity around the z-axis in degrees per second.
  constexpr void updateAcceleration(float ax, float ay, float az, float gx,
                                    float gy, float gz)
  {
    std::apply(
        [=](auto&... callback) {
          (callback.onAccelerationUpdate(ax, ay, az, gx, gy, gz), ...);
        },
        m_callbacks);
  }

  /// @brief Updates the altitude for telemetry transmission.
  ///
  /// @param altitude Altitude in meters.
  constexpr void updateAltitude(float altitude)
  {
    std::apply(
        [=](auto&... callback) { (callback.onAltitudeUpdate(altitude), ...); },
        m_callbacks);
  }

  /// @brief Updates the vertical velocity for telemetry transmission.
  ///
  /// @param verticalVelocity Vertical velocity in meters per second.
  constexpr void updateVerticalVelocity(float verticalVelocity)
  {
    std::apply(
        [=](auto&... callback) {
          (callback.onVerticalVelocityUpdate(verticalVelocity), ...);
        },
        m_callbacks);
  }

  /// @brief Updates the pressure readings from two barometers for telemetry
  /// transmission.
  ///
  /// @param pressure1 Pressure reading from the first barometer in mBar.
  /// @param pressure2 Pressure reading from the second barometer in mBar.
  constexpr void updatePressure(float pressure1, float pressure2)
  {
    std::apply(
        [=](auto&... callback) {
          (callback.onPressureUpdate(pressure1, pressure2), ...);
        },
        m_callbacks);
  }

  /// @brief Updates the temperature readings for telemetry transmission.
  ///
  /// @param temperature1 Temperature reading from the first sensor in degrees.
  /// @param temperature2 Temperature reading from the second sensor in degrees.
  constexpr void updateTemperature(float temperature1, float temperature2)
  {
    std::apply(
        [=](auto&... callback) {
          (callback.onTemperatureUpdate(temperature1, temperature2), ...);
        },
        m_callbacks);
  }

  /// @brief Updates the airbrake extension
  ///
  /// @param extension Airbrake extension percentage [0.0, 1.0]
  constexpr void updateAirbrakeExtension(float extension)
  {
    std::apply(
        [=](auto&... callback) {
          (callback.onAirbrakeExtensionUpdate(extension), ...);
        },
        m_callbacks);
  }

private:
  std::tuple<Callbacks...> m_callbacks;
};
} // namespace mcu::telemetry