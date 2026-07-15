#include "Telemetry.h"

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
  m_context.push_back(callback.context);
  m_onFlightStatusUpdate.push_back(callback.onFlightStatusUpdate);
  m_onLinkStatusUpdate.push_back(callback.onLinkStatusUpdate);
  m_onAttitudeUpdate.push_back(callback.onAttitudeUpdate);
  m_onMapPositionUpdate.push_back(callback.onMapPositionUpdate);
  m_onAccelerationUpdate.push_back(callback.onAccelerationUpdate);
  m_onAltitudeUpdate.push_back(callback.onAltitudeUpdate);
  m_onVerticalVelocityUpdate.push_back(callback.onVerticalVelocityUpdate);
  m_onPressureUpdate.push_back(callback.onPressureUpdate);
  m_onTemperatureUpdate.push_back(callback.onTemperatureUpdate);
  m_onAirbrakeExtensionUpdate.push_back(callback.onAirbrakeExtensionUpdate);
}

void Manager::popBackCallback()
{
  assert(!m_context.empty() && "No callbacks to remove");

  m_context.pop_back();
  m_onFlightStatusUpdate.pop_back();
  m_onLinkStatusUpdate.pop_back();
  m_onAttitudeUpdate.pop_back();
  m_onMapPositionUpdate.pop_back();
  m_onAccelerationUpdate.pop_back();
  m_onAltitudeUpdate.pop_back();
  m_onVerticalVelocityUpdate.pop_back();
  m_onPressureUpdate.pop_back();
  m_onTemperatureUpdate.pop_back();
  m_onAirbrakeExtensionUpdate.pop_back();
}

void Manager::updateFlightStatus(FlightStatus status)
{
  for (size_t i = 0; i < m_onFlightStatusUpdate.size(); ++i)
    m_onFlightStatusUpdate[i](status, m_context[i]);
}

void Manager::updateLinkStatus(bool connected, float rssi, float snr)
{
  for (size_t i = 0; i < m_onLinkStatusUpdate.size(); ++i)
    m_onLinkStatusUpdate[i](connected, rssi, snr, m_context[i]);
}

void Manager::updateAttitude(float roll, float pitch, float yaw)
{
  for (size_t i = 0; i < m_onAttitudeUpdate.size(); ++i)
    m_onAttitudeUpdate[i](roll, pitch, yaw, m_context[i]);
}

void Manager::updateMapPosition(double latitude, double longitude)
{
  for (size_t i = 0; i < m_onMapPositionUpdate.size(); ++i)
    m_onMapPositionUpdate[i](latitude, longitude, m_context[i]);
}

void Manager::updateAcceleration(float ax, float ay, float az, float gx,
                                 float gy, float gz)
{
  for (size_t i = 0; i < m_onAccelerationUpdate.size(); ++i)
    m_onAccelerationUpdate[i](ax, ay, az, gx, gy, gz, m_context[i]);
}

void Manager::updateAltitude(float altitude)
{
  for (size_t i = 0; i < m_onAltitudeUpdate.size(); ++i)
    m_onAltitudeUpdate[i](altitude, m_context[i]);
}

void Manager::updateVerticalVelocity(float verticalVelocity)
{
  for (size_t i = 0; i < m_onVerticalVelocityUpdate.size(); ++i)
    m_onVerticalVelocityUpdate[i](verticalVelocity, m_context[i]);
}

void Manager::updatePressure(float pressure1, float pressure2)
{
  for (size_t i = 0; i < m_onPressureUpdate.size(); ++i)
    m_onPressureUpdate[i](pressure1, pressure2, m_context[i]);
}

void Manager::updateTemperature(float temperature1, float temperature2)
{
  for (size_t i = 0; i < m_onTemperatureUpdate.size(); ++i)
    m_onTemperatureUpdate[i](temperature1, temperature2, m_context[i]);
}

void Manager::updateAirbrakeExtension(float extension)
{
  for (size_t i = 0; i < m_onAirbrakeExtensionUpdate.size(); ++i)
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