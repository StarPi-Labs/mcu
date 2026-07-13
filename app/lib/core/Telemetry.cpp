#include "Telemetry.h"

namespace mcu::telemetry
{
Manager::Manager()
    : m_callbacks()
{
}

Manager::Manager(std::initializer_list<Callback> callbacks)
    : m_callbacks(callbacks)
{
}

void Manager::pushCallback(const Callback& callback)
{
  assert(isValidCallback(callback) &&
         "Invalid callback: all function pointers must be non-nullptr");
  m_callbacks.push_back(callback);
}

void Manager::popBackCallback()
{
  assert(!m_callbacks.empty() && "No callbacks to remove");
  m_callbacks.pop_back();
}

void Manager::updateFlightStatus(FlightStatus status)
{
  for (const auto& callback : m_callbacks)
    callback.onFlightStatusUpdate(status, callback.context);
}

void Manager::updateLinkStatus(bool connected, float rssi, float snr)
{
  for (const auto& callback : m_callbacks)
    callback.onLinkStatusUpdate(connected, rssi, snr, callback.context);
}

void Manager::updateAttitude(float roll, float pitch, float yaw)
{
  for (const auto& callback : m_callbacks)
    callback.onAttitudeUpdate(roll, pitch, yaw, callback.context);
}

void Manager::updateMapPosition(double latitude, double longitude)
{
  for (const auto& callback : m_callbacks)
    callback.onMapPositionUpdate(latitude, longitude, callback.context);
}

void Manager::updateAcceleration(float ax, float ay, float az, float gx,
                                 float gy, float gz)
{
  for (const auto& callback : m_callbacks)
    callback.onAccelerationUpdate(ax, ay, az, gx, gy, gz, callback.context);
}

void Manager::updateAltitude(float altitude)
{
  for (const auto& callback : m_callbacks)
    callback.onAltitudeUpdate(altitude, callback.context);
}

void Manager::updateVerticalVelocity(float verticalVelocity)
{
  for (const auto& callback : m_callbacks)
    callback.onVerticalVelocityUpdate(verticalVelocity, callback.context);
}

void Manager::updatePressure(float pressure1, float pressure2)
{
  for (const auto& callback : m_callbacks)
    callback.onPressureUpdate(pressure1, pressure2, callback.context);
}

void Manager::updateTemperature(float temperature1, float temperature2)
{
  for (const auto& callback : m_callbacks)
    callback.onTemperatureUpdate(temperature1, temperature2, callback.context);
}

void Manager::updateAirbrakeExtension(float extension)
{
  for (const auto& callback : m_callbacks)
    callback.onAirbrakeExtensionUpdate(extension, callback.context);
}

bool Manager::isValidCallback(const Callback& callback)
{
  return callback.onFlightStatusUpdate != nullptr &&
         callback.onLinkStatusUpdate != nullptr &&
         callback.onAttitudeUpdate != nullptr &&
         callback.onMapPositionUpdate != nullptr &&
         callback.onAccelerationUpdate != nullptr &&
         callback.onAltitudeUpdate != nullptr &&
         callback.onVerticalVelocityUpdate != nullptr &&
         callback.onPressureUpdate != nullptr &&
         callback.onTemperatureUpdate != nullptr &&
         callback.onAirbrakeExtensionUpdate != nullptr;
}
} // namespace mcu::telemetry