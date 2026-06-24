#include <Arduino.h>
#include <format>
#include <string>
#include <unity.h>

#include <Telemetry.h>

void setUp() {}
void tearDown() {}

using namespace mcu::telemetry;

struct Result {
  FlightStatus flightStatus;
  float connected, rssi, snr;
  float roll, pitch, yaw;
  float latitude, longitude;
  float ax, ay, az, gx, gy, gz;
  float altitude;
  float verticalVelocity;
  float pressure1, pressure2;
  float temperature1, temperature2;
  float airbrakeExtension;

  bool equals(const Result& other) const
  {
    return flightStatus == other.flightStatus && connected == other.connected &&
           rssi == other.rssi && snr == other.snr && roll == other.roll &&
           pitch == other.pitch && yaw == other.yaw &&
           latitude == other.latitude && longitude == other.longitude &&
           ax == other.ax && ay == other.ay && az == other.az &&
           gx == other.gx && gy == other.gy && gz == other.gz &&
           altitude == other.altitude &&
           verticalVelocity == other.verticalVelocity &&
           pressure1 == other.pressure1 && pressure2 == other.pressure2 &&
           temperature1 == other.temperature1 &&
           temperature2 == other.temperature2 &&
           airbrakeExtension == other.airbrakeExtension;
  }

  std::string toString() const
  {
    return std::format(
        "{{ flightStatus: {}, linkStatus: {{ connected: {}, rssi: {}, snr: {} "
        "}}, roll: {}, pitch: {}, yaw: {}, latitude: {}, longitude: {}, ax: "
        "{}, ay: {}, az: {}, gx: {}, gy: {}, gz: {}, altitude: {}, "
        "verticalVelocity: {}, pressure1: {}, pressure2: {}, temperature1: {}, "
        "temperature2: {}, airbrakeExtension: {} }}",
        static_cast<int>(flightStatus), connected, rssi, snr, roll, pitch, yaw,
        latitude, longitude, ax, ay, az, gx, gy, gz, altitude, verticalVelocity,
        pressure1, pressure2, temperature1, temperature2, airbrakeExtension);
  }
};

static Result g_testResult;

class TestCallback
{
public:
  void onFlightStatusUpdate(FlightStatus status)
  {
    g_testResult.flightStatus = status;
  }

  void onLinkStatusUpdate(bool connected, float rssi, float snr)
  {
    g_testResult.connected = connected;
    g_testResult.rssi = rssi;
    g_testResult.snr = snr;
  }

  void onAttitudeUpdate(float roll, float pitch, float yaw)
  {
    g_testResult.roll = roll;
    g_testResult.pitch = pitch;
    g_testResult.yaw = yaw;
  }

  void onMapPositionUpdate(float latitude, float longitude)
  {
    g_testResult.latitude = latitude;
    g_testResult.longitude = longitude;
  }

  void onAccelerationUpdate(float ax, float ay, float az, float gx, float gy,
                            float gz)
  {
    g_testResult.ax = ax;
    g_testResult.ay = ay;
    g_testResult.az = az;
    g_testResult.gx = gx;
    g_testResult.gy = gy;
    g_testResult.gz = gz;
  }

  void onAltitudeUpdate(float altitude) { g_testResult.altitude = altitude; }

  void onVerticalVelocityUpdate(float verticalVelocity)
  {
    g_testResult.verticalVelocity = verticalVelocity;
  }

  void onPressureUpdate(float pressure1, float pressure2)
  {
    g_testResult.pressure1 = pressure1;
    g_testResult.pressure2 = pressure2;
  }

  void onTemperatureUpdate(float temperature1, float temperature2)
  {
    g_testResult.temperature1 = temperature1;
    g_testResult.temperature2 = temperature2;
  }

  void onAirbrakeExtensionUpdate(float extension)
  {
    g_testResult.airbrakeExtension = extension;
  }
};

void test_telemetry_manager(void)
{
  Manager<TestCallback> manager;

  Result expected{FlightStatus::DESCENT,
                  true,
                  -65.0f,
                  8.0f,
                  15.0f,
                  25.0f,
                  35.0f,
                  38.0f,
                  -122.0f,
                  1.5f,
                  2.5f,
                  3.5f,
                  4.5f,
                  5.5f,
                  6.5f,
                  1500.0f,
                  -10.0f,
                  1012.0f,
                  1011.5f,
                  26.0f,
                  25.5f,
                  .75f};

  manager.updateFlightStatus(expected.flightStatus);
  manager.updateLinkStatus(expected.connected, expected.rssi, expected.snr);
  manager.updateAttitude(expected.roll, expected.pitch, expected.yaw);
  manager.updateMapPosition(expected.latitude, expected.longitude);
  manager.updateAcceleration(expected.ax, expected.ay, expected.az, expected.gx,
                             expected.gy, expected.gz);
  manager.updateAltitude(expected.altitude);
  manager.updateVerticalVelocity(expected.verticalVelocity);
  manager.updatePressure(expected.pressure1, expected.pressure2);
  manager.updateTemperature(expected.temperature1, expected.temperature2);
  manager.updateAirbrakeExtension(expected.airbrakeExtension);

  TEST_ASSERT_TRUE_MESSAGE(g_testResult.equals(expected),
                           std::format("Expected: {}, Got: {}",
                                       expected.toString(),
                                       g_testResult.toString())
                               .c_str());
}

void setup()
{
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_telemetry_manager);
  UNITY_END();
}

void loop() {}