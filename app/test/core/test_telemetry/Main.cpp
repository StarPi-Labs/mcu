#include <Arduino.h>
#include <format>
#include <string>
#include <unity.h>

#include <Telemetry.h>

void setUp() {}
void tearDown() {}

static mcu::telemetry::Manager g_manager;

static mcu::telemetry::FlightStatus g_flightStatus;
static bool g_connected;
static float g_rssi;
static float g_snr;
static float g_roll;
static float g_pitch;
static float g_yaw;
static double g_latitude;
static double g_longitude;
static float g_ax;
static float g_ay;
static float g_az;
static float g_gx;
static float g_gy;
static float g_gz;
static float g_altitude;
static float g_verticalVelocity;
static float g_pressure1;
static float g_pressure2;
static float g_temperature1;
static float g_temperature2;
static float g_airbrakeExtension;

void test_update_functions(void)
{
  using namespace mcu::telemetry;

  constexpr Manager::Callback callback = {
      nullptr,
      [](FlightStatus status, void* context) { g_flightStatus = status; },
      [](bool connected, float rssi, float snr, void* context) {
        g_connected = connected;
        g_rssi = rssi;
        g_snr = snr;
      },
      [](float roll, float pitch, float yaw, void* context) {
        g_roll = roll;
        g_pitch = pitch;
        g_yaw = yaw;
      },
      [](double latitude, double longitude, void* context) {
        g_latitude = latitude;
        g_longitude = longitude;
      },
      [](float ax, float ay, float az, float gx, float gy, float gz,
         void* context) {
        g_ax = ax;
        g_ay = ay;
        g_az = az;
        g_gx = gx;
        g_gy = gy;
        g_gz = gz;
      },
      [](float altitude, void* context) { g_altitude = altitude; },
      [](float verticalVelocity, void* context) {
        g_verticalVelocity = verticalVelocity;
      },
      [](float pressure1, float pressure2, void* context) {
        g_pressure1 = pressure1;
        g_pressure2 = pressure2;
      },
      [](float temperature1, float temperature2, void* context) {
        g_temperature1 = temperature1;
        g_temperature2 = temperature2;
      },
      [](float extension, void* context) { g_airbrakeExtension = extension; }};

  g_manager.pushBackCallback(callback);

  {
    constexpr FlightStatus EXPECTED = FlightStatus::BOOST;
    g_manager.updateFlightStatus(EXPECTED);
    TEST_ASSERT_EQUAL_INT(EXPECTED, g_flightStatus);
  }

  {
    constexpr struct {
      bool connected;
      float rssi;
      float snr;
    } EXPECTED = {true, -70.0f, 5.0f};

    g_manager.updateLinkStatus(EXPECTED.connected, EXPECTED.rssi, EXPECTED.snr);

    TEST_ASSERT_EQUAL_INT(EXPECTED.connected, g_connected);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.rssi, g_rssi);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.snr, g_snr);
  }

  {
    constexpr struct {
      float roll;
      float pitch;
      float yaw;
    } EXPECTED = {10.0f, 20.0f, 30.0f};

    g_manager.updateAttitude(EXPECTED.roll, EXPECTED.pitch, EXPECTED.yaw);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.roll, g_roll);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.pitch, g_pitch);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.yaw, g_yaw);
  }

  {
    constexpr struct {
      double latitude;
      double longitude;
    } EXPECTED = {37.7749, -122.4194};

    g_manager.updateMapPosition(EXPECTED.latitude, EXPECTED.longitude);

    TEST_ASSERT_FLOAT_WITHIN(0.001, EXPECTED.latitude, g_latitude);
    TEST_ASSERT_FLOAT_WITHIN(0.001, EXPECTED.longitude, g_longitude);
  }

  {
    constexpr struct {
      float ax;
      float ay;
      float az;
      float gx;
      float gy;
      float gz;
    } EXPECTED = {0.1f, 0.2f, 0.3f, 1.0f, 2.0f, 3.0f};
    g_manager.updateAcceleration(EXPECTED.ax, EXPECTED.ay, EXPECTED.az,
                                 EXPECTED.gx, EXPECTED.gy, EXPECTED.gz);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.ax, g_ax);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.ay, g_ay);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.az, g_az);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.gx, g_gx);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.gy, g_gy);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.gz, g_gz);
  }

  {
    constexpr float EXPECTED_ALTITUDE = 1000.0f;
    g_manager.updateAltitude(EXPECTED_ALTITUDE);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED_ALTITUDE, g_altitude);
  }

  {
    constexpr float EXPECTED_VERTICAL_VELOCITY = 50.0f;
    g_manager.updateVerticalVelocity(EXPECTED_VERTICAL_VELOCITY);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED_VERTICAL_VELOCITY,
                             g_verticalVelocity);
  }

  {
    constexpr struct {
      float pressure1;
      float pressure2;
    } EXPECTED = {1013.25f, 1012.75f};
    g_manager.updatePressure(EXPECTED.pressure1, EXPECTED.pressure2);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.pressure1, g_pressure1);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.pressure2, g_pressure2);
  }

  {
    constexpr struct {
      float temperature1;
      float temperature2;
    } EXPECTED = {25.0f, 24.5f};
    g_manager.updateTemperature(EXPECTED.temperature1, EXPECTED.temperature2);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.temperature1, g_temperature1);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.temperature2, g_temperature2);
  }

  {
    constexpr float EXPECTED_AIRBRAKE_EXTENSION = 0.75f;
    g_manager.updateAirbrakeExtension(EXPECTED_AIRBRAKE_EXTENSION);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED_AIRBRAKE_EXTENSION,
                             g_airbrakeExtension);
  }

  {
    constexpr struct {
      float temperature1;
      float temperature2;
    } EXPECTED = {25.0f, 24.5f};
    g_manager.updateTemperature(EXPECTED.temperature1, EXPECTED.temperature2);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.temperature1, g_temperature1);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.temperature2, g_temperature2);
  }

  {
    constexpr struct {
      float airbrakeExtension;
    } EXPECTED = {0.75f};
    g_manager.updateAirbrakeExtension(EXPECTED.airbrakeExtension);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, EXPECTED.airbrakeExtension,
                             g_airbrakeExtension);
  }
}

void test_remove_callback(void)
{
  g_manager.updateFlightStatus(mcu::telemetry::FlightStatus::IDLE);

  g_manager.popBackCallback();

  // Update should do nothing
  g_manager.updateFlightStatus(mcu::telemetry::FlightStatus::BOOST);

  TEST_ASSERT_EQUAL_INT(mcu::telemetry::FlightStatus::IDLE, g_flightStatus);
}

void setup()
{
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_update_functions);
  RUN_TEST(test_remove_callback);
  UNITY_END();
}

void loop() {}