// @file Bluetooth.cpp
/// @brief Implementation for Bluetooth telemetry
/// @authors Adnaan Juma
/// @version 1.0

#include "Bluetooth.h"
#include "CoreConfig.h"
#include "Logging.h"
#include "NimBLEServer.h"

namespace mcu::bluetooth
{
/// ---------------------------------------------------------------------------
/// ------------------------------- CONSTANTS ---------------------------------
/// ---------------------------------------------------------------------------

static constexpr uint32_t READ_NOTIFY_PROP =
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY;

static const NimBLEUUID USER_DESC_UUID((uint16_t)0x2901);

static const NimBLEUUID
    FLIGHT_CONTROL_SERVICE_UUID("0e18c423-db80-4695-babb-c04dc119ef0f");

static const NimBLEUUID
    FLIGHT_STATUS_CHARACTERISTIC_UUID("0e18c423-db80-4696-babb-c04dc119ef0f");

static constexpr const char* FLIGHT_STATUS_DESCRIPTION =
    "JSON: [ <flight status (number)> ]\n"
    "Possible values: IDLE = 0, ARMED = 1, BOOST = 2, COAST = 3, AIRBRAKE = 4, "
    "DEPLOY = 5, DESCENT = 6";

static const NimBLEUUID
    LINK_STATUS_CHARACTERISTIC_UUID("0e18c423-db80-4697-babb-c04dc119ef0f");

static constexpr const char* LINK_STATUS_DESCRIPTION =
    "JSON: [ <connection (bool)>, <rssi (number)>, <snr (number)> ]";

static const NimBLEUUID
    NAVIGATION_CONTROL_SERVICE_UUID("0e18c423-db80-4698-babb-c04dc119ef0f");

static const NimBLEUUID
    ATTITUDE_CHARACTERISTIC_UUID("0e18c423-db80-4699-babb-c04dc119ef0f");

static constexpr const char* ATTITUDE_DESCRIPTION =
    "JSON: [ <roll (number)>, <pitch (number)>, <yaw (number)>\n"
    "All in degrees";

static const NimBLEUUID
    MAP_POSITION_CHARACTERISTIC_UUID("0e18c423-db80-469a-babb-c04dc119ef0f");

static constexpr const char* MAP_POSITION_DESCRIPTION =
    "JSON: [ <latitude (number)>, <longitude (number)> ]\n"
    "All in degrees";

static const NimBLEUUID
    ACCELERATION_CHARACTERISTIC_UUID("0e18c423-db80-469b-babb-c04dc119ef0f");

static constexpr const char* ACCELERATION_DESCRIPTION =
    "JSON: [ <ax (number)>, <ay (number)>, <az (number)>, <gx (number)>, "
    "<gy (number)>, <gz (number)> ]\n"
    "Accelerations in m/s^2 and angular velocities in rad/s";

static const NimBLEUUID
    ALTITUDE_CHARACTERISTIC_UUID("0e18c423-db80-469c-babb-c04dc119ef0f");

static constexpr const char* ALTITUDE_DESCRIPTION =
    "JSON: <altitude (number)>\n"
    "In meters";

static const NimBLEUUID VERTICAL_VELOCITY_CHARACTERISTIC_UUID(
    "0e18c423-db80-469d-babb-c04dc119ef0f");

static constexpr const char* VERTICAL_VELOCITY_DESCRIPTION =
    "JSON: <vertical velocity (number)>\n"
    "Estimation in m/s";

static const NimBLEUUID
    ENVIRONMENTAL_CONTROL_SERVICE_UUID("0e18c423-db80-469e-babb-c04dc119ef0f");

static const NimBLEUUID
    PRESSURE_CHARACTERISTIC_UUID("0e18c423-db80-469f-babb-c04dc119ef0f");

static constexpr const char* PRESSURE_DESCRIPTION =
    "JSON: [ <pressure 1 (number)>, <pressure 2 (number)> ]\n"
    "All in mBar";

static const NimBLEUUID
    TEMPERATURE_CHARACTERISTIC_UUID("0e18c423-db80-46a0-babb-c04dc119ef0f");

static constexpr const char* TEMPERATURE_DESCRIPTION =
    "JSON: [ <temperature 1 (number)>, <temperature 2 (number)> ]\n"
    "All in C";

static const NimBLEUUID
    ACTUATOR_CONTROL_SERVICE_UUID("0e18c423-db80-46a1-babb-c04dc119ef0f");

static const NimBLEUUID AIRBRAKE_EXTENSION_CHARACTERISTIC_UUID(
    "0e18c423-db80-46a2-babb-c04dc119ef0f");

static constexpr const char* AIRBRAKE_EXTENSION_DESCRIPTION =
    "JSON: [ <airbrake extension (number)> ]\n"
    "In [0.0, 1.0]";

/// ---------------------------------------------------------------------------
/// -------------------------- MANAGER CALLBACK -------------------------------
/// ---------------------------------------------------------------------------
mcu::telemetry::Manager::Callback Manager::getTelemetryCallback()
{
  using namespace mcu::telemetry;

  return {
      this,
      [](FlightStatus status, void* context) {
        bluetooth::Manager* self =
            reinterpret_cast<bluetooth::Manager*>(context);

        self->updateFlightStatus(status);
        if (!self->m_pFlightStatus->notify())
          mcu_log_warning("bluetooth",
                          "Failed to notify on flight status update");
      },
      [](bool connected, float rssi, float snr, void* context) {
        bluetooth::Manager* self =
            reinterpret_cast<bluetooth::Manager*>(context);

        self->updateLinkStatus(connected, rssi, snr);
        if (!self->m_pLinkStatus->notify())
          mcu_log_warning("bluetooth",
                          "Failed to notify on link status update");
      },
      [](float roll, float pitch, float yaw, void* context) {
        bluetooth::Manager* self =
            reinterpret_cast<bluetooth::Manager*>(context);

        self->updateAttitude(roll, pitch, yaw);
        if (!self->m_pAttitude->notify())
          mcu_log_warning("bluetooth", "Failed to notify on attitude update");
      },
      [](double latitude, double longitude, void* context) {
        bluetooth::Manager* self =
            reinterpret_cast<bluetooth::Manager*>(context);

        self->updateMapPosition(latitude, longitude);
        if (!self->m_pMapPosition->notify())
          mcu_log_warning("bluetooth",
                          "Failed to notify on map position update");
      },
      [](float ax, float ay, float az, float gx, float gy, float gz,
         void* context) {
        bluetooth::Manager* self =
            reinterpret_cast<bluetooth::Manager*>(context);

        self->updateAcceleration(ax, ay, az, gx, gy, gz);
        if (!self->m_pAcceleration->notify())
          mcu_log_warning("bluetooth",
                          "Failed to notify on acceleration update");
      },
      [](float altitude, void* context) {
        bluetooth::Manager* self =
            reinterpret_cast<bluetooth::Manager*>(context);

        self->updateAltitude(altitude);
        if (!self->m_pAltitude->notify())
          mcu_log_warning("bluetooth", "Failed to notify on altitude update");
      },
      [](float verticalVelocity, void* context) {
        bluetooth::Manager* self =
            reinterpret_cast<bluetooth::Manager*>(context);

        self->updateVerticalVelocity(verticalVelocity);
        if (!self->m_pVerticalVelocity->notify())
          mcu_log_warning("bluetooth",
                          "Failed to notify on vertical velocity update");
      },
      [](float pressure1, float pressure2, void* context) {
        bluetooth::Manager* self =
            reinterpret_cast<bluetooth::Manager*>(context);

        self->updatePressure(pressure1, pressure2);
        if (!self->m_pPressure->notify())
          mcu_log_warning("bluetooth", "Failed to notify on pressure update");
      },
      [](float temperature1, float temperature2, void* context) {
        bluetooth::Manager* self =
            reinterpret_cast<bluetooth::Manager*>(context);

        self->updateTemperature(temperature1, temperature2);
        if (!self->m_pTemperature->notify())
          mcu_log_warning("bluetooth",
                          "Failed to notify on temperature update");
      },
      [](float extension, void* context) {
        bluetooth::Manager* self =
            reinterpret_cast<bluetooth::Manager*>(context);

        self->updateAirbrakeExtension(extension);
        if (!self->m_pAirbrakeExtension->notify())
          mcu_log_warning("bluetooth",
                          "Failed to notify on airbrake extension update");
      }};
}

/// ---------------------------------------------------------------------------
/// ------------------------------ MANAGER ------------------------------------
/// ---------------------------------------------------------------------------

void Manager::updateFlightStatus(mcu::telemetry::FlightStatus status)
{
  assert(m_pFlightStatus && "Flight status characteristic not initialized");
  std::string str = std::format("{}", static_cast<std::uint8_t>(status));
  m_pFlightStatus->setValue(str);
}

void Manager::updateLinkStatus(bool connected, float rssi, float snr)
{
  assert(m_pLinkStatus && "Link status characteristic not initialized");
  std::string str =
      std::format("[{}, {}, {}]", connected ? "true" : "false", rssi, snr);
  m_pLinkStatus->setValue(str);
}

void Manager::updateAttitude(float roll, float pitch, float yaw)
{
  assert(m_pAttitude && "Attitude characteristic not initialized");
  std::string str = std::format("[{}, {}, {}]", roll, pitch, yaw);
  m_pAttitude->setValue(str);
}

void Manager::updateMapPosition(double latitude, double longitude)
{
  assert(m_pMapPosition && "Map position characteristic not initialized");
  std::string str = std::format("[{}, {}]", latitude, longitude);
  m_pMapPosition->setValue(str);
}

void Manager::updateAcceleration(float ax, float ay, float az, float gx,
                                 float gy, float gz)
{
  assert(m_pAcceleration && "Acceleration characteristic not initialized");
  std::string str =
      std::format("[{}, {}, {}, {}, {}, {}]", ax, ay, az, gx, gy, gz);
  m_pAcceleration->setValue(str);
}

void Manager::updateAltitude(float altitude)
{
  assert(m_pAltitude && "Altitude characteristic not initialized");
  std::string str = std::format("{}", altitude);
  m_pAltitude->setValue(str);
}

void Manager::updateVerticalVelocity(float verticalVelocity)
{
  assert(m_pVerticalVelocity &&
         "Vertical velocity characteristic not initialized");
  std::string str = std::format("{}", verticalVelocity);
  m_pVerticalVelocity->setValue(str);
}

void Manager::updatePressure(float pressure1, float pressure2)
{
  assert(m_pPressure && "Pressure characteristic not initialized");
  std::string str = std::format("[{}, {}]", pressure1, pressure2);
  m_pPressure->setValue(str);
}

void Manager::updateTemperature(float temperature1, float temperature2)
{
  assert(m_pTemperature && "Temperature characteristic not initialized");
  std::string str = std::format("[{}, {}]", temperature1, temperature2);
  m_pTemperature->setValue(str);
}

void Manager::updateAirbrakeExtension(float extension)
{
  assert(m_pAirbrakeExtension &&
         "Airbrake extension characteristic not initialized");
  std::string str = std::format("{}", extension);
  m_pAirbrakeExtension->setValue(str);
}

void Manager::initFlightControlService(NimBLEServer& server)
{
  NimBLEService* pService = server.createService(FLIGHT_CONTROL_SERVICE_UUID);

  m_pFlightStatus = pService->createCharacteristic(
      FLIGHT_STATUS_CHARACTERISTIC_UUID, READ_NOTIFY_PROP);
  // Defaults to IDLE
  updateFlightStatus(mcu::telemetry::FlightStatus::IDLE);
  m_pFlightStatus->create2904()->setFormat(NimBLE2904::FORMAT_UTF8);
  m_pFlightStatus->createDescriptor(USER_DESC_UUID, NIMBLE_PROPERTY::READ)
      ->setValue(FLIGHT_STATUS_DESCRIPTION);

  m_pLinkStatus = pService->createCharacteristic(
      LINK_STATUS_CHARACTERISTIC_UUID, READ_NOTIFY_PROP);
  // Defaults to disconnected, and 0 for rssi and snr
  updateLinkStatus(false, 0.0f, 0.0f);
  m_pLinkStatus->create2904()->setFormat(NimBLE2904::FORMAT_UTF8);
  m_pLinkStatus->createDescriptor(USER_DESC_UUID, NIMBLE_PROPERTY::READ)
      ->setValue(LINK_STATUS_DESCRIPTION);
}

void Manager::initNavigationControlService(NimBLEServer& server)
{
  NimBLEService* pService =
      server.createService(NAVIGATION_CONTROL_SERVICE_UUID);

  m_pAttitude = pService->createCharacteristic(ATTITUDE_CHARACTERISTIC_UUID,
                                               READ_NOTIFY_PROP);
  // Defaults to 0 for roll, pitch, and yaw
  updateAttitude(0.0f, 0.0f, 0.0f);
  m_pAttitude->create2904()->setFormat(NimBLE2904::FORMAT_UTF8);
  m_pAttitude->createDescriptor(USER_DESC_UUID, NIMBLE_PROPERTY::READ)
      ->setValue(ATTITUDE_DESCRIPTION);

  m_pMapPosition = pService->createCharacteristic(
      MAP_POSITION_CHARACTERISTIC_UUID, READ_NOTIFY_PROP);
  // Defaults to 0 for latitude and longitude
  updateMapPosition(0.0, 0.0);
  m_pMapPosition->create2904()->setFormat(NimBLE2904::FORMAT_UTF8);
  m_pMapPosition->createDescriptor(USER_DESC_UUID, NIMBLE_PROPERTY::READ)
      ->setValue(MAP_POSITION_DESCRIPTION);

  m_pAcceleration = pService->createCharacteristic(
      ACCELERATION_CHARACTERISTIC_UUID, READ_NOTIFY_PROP);
  // Defaults to 0 for all acceleration and angular velocity values
  updateAcceleration(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  m_pAcceleration->create2904()->setFormat(NimBLE2904::FORMAT_UTF8);
  m_pAcceleration->createDescriptor(USER_DESC_UUID, NIMBLE_PROPERTY::READ)
      ->setValue(ACCELERATION_DESCRIPTION);

  m_pAltitude = pService->createCharacteristic(ALTITUDE_CHARACTERISTIC_UUID,
                                               READ_NOTIFY_PROP);
  // Defaults to 0 for altitude
  updateAltitude(0.0f);
  m_pAltitude->create2904()->setFormat(NimBLE2904::FORMAT_UTF8);
  m_pAltitude->createDescriptor(USER_DESC_UUID, NIMBLE_PROPERTY::READ)
      ->setValue(ALTITUDE_DESCRIPTION);

  m_pVerticalVelocity = pService->createCharacteristic(
      VERTICAL_VELOCITY_CHARACTERISTIC_UUID, READ_NOTIFY_PROP);
  // Defaults to 0 for vertical velocity
  updateVerticalVelocity(0.0f);
  m_pVerticalVelocity->create2904()->setFormat(NimBLE2904::FORMAT_UTF8);
  m_pVerticalVelocity->createDescriptor(USER_DESC_UUID, NIMBLE_PROPERTY::READ)
      ->setValue(VERTICAL_VELOCITY_DESCRIPTION);
}

void Manager::initEnvironmentalControlService(NimBLEServer& server)
{
  NimBLEService* pService =
      server.createService(ENVIRONMENTAL_CONTROL_SERVICE_UUID);

  m_pPressure = pService->createCharacteristic(PRESSURE_CHARACTERISTIC_UUID,
                                               READ_NOTIFY_PROP);
  // Defaults to 0 for both pressure values
  updatePressure(0.0f, 0.0f);
  m_pPressure->create2904()->setFormat(NimBLE2904::FORMAT_UTF8);
  m_pPressure->createDescriptor(USER_DESC_UUID, NIMBLE_PROPERTY::READ)
      ->setValue(PRESSURE_DESCRIPTION);

  m_pTemperature = pService->createCharacteristic(
      TEMPERATURE_CHARACTERISTIC_UUID, READ_NOTIFY_PROP);
  // Defaults to 0 for both temperature values
  updateTemperature(0.0f, 0.0f);
  m_pTemperature->create2904()->setFormat(NimBLE2904::FORMAT_UTF8);
  m_pTemperature->createDescriptor(USER_DESC_UUID, NIMBLE_PROPERTY::READ)
      ->setValue(TEMPERATURE_DESCRIPTION);
}

void Manager::initActuatorControlService(NimBLEServer& server)
{
  NimBLEService* pService = server.createService(ACTUATOR_CONTROL_SERVICE_UUID);

  m_pAirbrakeExtension = pService->createCharacteristic(
      AIRBRAKE_EXTENSION_CHARACTERISTIC_UUID, READ_NOTIFY_PROP);
  // Defaults to 0 for airbrake extension
  updateAirbrakeExtension(0.0f);
  m_pAirbrakeExtension->create2904()->setFormat(NimBLE2904::FORMAT_UTF8);
  m_pAirbrakeExtension->createDescriptor(USER_DESC_UUID, NIMBLE_PROPERTY::READ)
      ->setValue(AIRBRAKE_EXTENSION_DESCRIPTION);
}

Manager::Manager()
    : m_pFlightStatus(nullptr),
      m_pLinkStatus(nullptr),
      m_pAttitude(nullptr),
      m_pMapPosition(nullptr),
      m_pAcceleration(nullptr),
      m_pAltitude(nullptr),
      m_pVerticalVelocity(nullptr),
      m_pPressure(nullptr),
      m_pTemperature(nullptr),
      m_pAirbrakeExtension(nullptr)
{
  bool success = NimBLEDevice::init(MCU_BLE_DEVICE_NAME);
  assert(success && "Failed to initialize NimBLEDevice");

  // Docs don't specify to check for nullptr
  NimBLEServer* pServer = NimBLEDevice::createServer();

  initFlightControlService(*pServer);
  initNavigationControlService(*pServer);
  initEnvironmentalControlService(*pServer);
  initActuatorControlService(*pServer);

  pServer->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();

  pAdvertising->enableScanResponse(true);
  success = pAdvertising->setName(MCU_BLE_DEVICE_NAME);
  assert(success && "Failed to set advertising name");

  success = pAdvertising->start();
  assert(success && "Failed to start advertising");
}

std::uint8_t Manager::getConnectedClients() const
{
  NimBLEServer* pServer = NimBLEDevice::getServer();
  assert(pServer && "NimBLE server not initialized");
  return pServer->getConnectedCount();
}

Manager::~Manager()
{
  // Clean up the NimBLE server and deinitialize the NimBLE device
  NimBLEDevice::deinit(true);
}
} // namespace mcu::bluetooth