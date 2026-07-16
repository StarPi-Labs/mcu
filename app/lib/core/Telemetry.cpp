#include "Telemetry.h"
#include <cassert>

namespace mcu::telemetry
{
Manager::Manager() {}

Manager::Manager(std::initializer_list<Callback> callbacks)
{
  for (const auto& cb : callbacks)
    pushBackCallback(cb);
}

void Manager::pushBackCallback(const Callback& callback)
{
  assert(isValidCallback(callback) &&
         "Invalid callback: all function pointers must be non-nullptr");
  assert(m_callbackCount < MCU_TELEMETRY_MAX_CALLBACKS &&
         "Maximum number of callbacks reached");

  m_context[m_callbackCount] = callback.context;
  m_onFlightStatusUpdate[m_callbackCount] = callback.onFlightStatusUpdate;
  m_onLinkStatusUpdate[m_callbackCount] = callback.onLinkStatusUpdate;
  m_onAttitudeUpdate[m_callbackCount] = callback.onAttitudeUpdate;
  m_onMapPositionUpdate[m_callbackCount] = callback.onMapPositionUpdate;
  m_onAccelerationUpdate[m_callbackCount] = callback.onAccelerationUpdate;
  m_onAltitudeUpdate[m_callbackCount] = callback.onAltitudeUpdate;
  m_onVerticalVelocityUpdate[m_callbackCount] = callback.onVerticalVelocityUpdate;
  m_onPressureUpdate[m_callbackCount] = callback.onPressureUpdate;
  m_onTemperatureUpdate[m_callbackCount] = callback.onTemperatureUpdate;
  m_onAirbrakeExtensionUpdate[m_callbackCount] = callback.onAirbrakeExtensionUpdate;

  ++m_callbackCount;
}

void Manager::popBackCallback()
{
  assert(m_callbackCount > 0 && "No callbacks to remove");
  --m_callbackCount;
}

void Manager::updateFlightStatus(FlightStatus status)
{
  for (size_t i = 0; i < m_callbackCount; ++i)
    m_onFlightStatusUpdate[i](status, m_context[i]);
}

void Manager::updateLinkStatus(bool connected, float rssi, float snr)
{
  for (size_t i = 0; i < m_callbackCount; ++i)
    m_onLinkStatusUpdate[i](connected, rssi, snr, m_context[i]);
}

void Manager::updateAttitude(float roll, float pitch, float yaw)
{
  for (size_t i = 0; i < m_callbackCount; ++i)
    m_onAttitudeUpdate[i](roll, pitch, yaw, m_context[i]);
}

void Manager::updateMapPosition(double latitude, double longitude)
{
  for (size_t i = 0; i < m_callbackCount; ++i)
    m_onMapPositionUpdate[i](latitude, longitude, m_context[i]);
}

void Manager::updateAcceleration(float ax, float ay, float az, float gx,
                                 float gy, float gz)
{
  for (size_t i = 0; i < m_callbackCount; ++i)
    m_onAccelerationUpdate[i](ax, ay, az, gx, gy, gz, m_context[i]);
}

void Manager::updateAltitude(float altitude)
{
  for (size_t i = 0; i < m_callbackCount; ++i)
    m_onAltitudeUpdate[i](altitude, m_context[i]);
}

void Manager::updateVerticalVelocity(float verticalVelocity)
{
  for (size_t i = 0; i < m_callbackCount; ++i)
    m_onVerticalVelocityUpdate[i](verticalVelocity, m_context[i]);
}

void Manager::updatePressure(float pressure1, float pressure2)
{
  for (size_t i = 0; i < m_callbackCount; ++i)
    m_onPressureUpdate[i](pressure1, pressure2, m_context[i]);
}

void Manager::updateTemperature(float temperature1, float temperature2)
{
  for (size_t i = 0; i < m_callbackCount; ++i)
    m_onTemperatureUpdate[i](temperature1, temperature2, m_context[i]);
}

void Manager::updateAirbrakeExtension(float extension)
{
  for (size_t i = 0; i < m_callbackCount; ++i)
    m_onAirbrakeExtensionUpdate[i](extension, m_context[i]);
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