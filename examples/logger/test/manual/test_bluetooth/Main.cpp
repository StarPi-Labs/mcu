#include <unity.h>

#include <Arduino.h>
#include <Bluetooth.hpp>

static bool g_called = false;

static BLECallbacks_t g_testCallbacks = {
    .on_sensor_calibration = [](void *context) { g_called = true; },
    .context = nullptr,
};

void setUp(void) { ; }

void tearDown(void) { ; }

void test_ble_init(void) {
  BLEStatus status = ble_init(g_testCallbacks);
  TEST_ASSERT_EQUAL(BLEStatus::SUCCESS, status);
}

void test_ble_send_log_message(void) {
  LogMessage message;
  message.timestamp = 0x123456789ABCDEF0;
  message.payload_type = P_STRING;
  message.src = S_OTHER;
  message.type = T_SYSLOG;
  message.payload.s = "This should read: F0-DE-BC-9A-78-56-34-12-08-03-...";

  BLEStatus status = ble_send_log_message(message);

  // Wait for a manual operator to connect and check
  while (ble_get_connection_count() == 0) {
    ;
  }

  // Wait until they disconnect
  while (ble_get_connection_count() != 0) {
    ;
  }

  TEST_ASSERT_EQUAL(BLEStatus::SUCCESS, status);
}

void test_ble_sensor_calibration_callback(void) {
  // Simulate a sensor calibration event

  // Wait for a manual operator to connect and write to the calibration
  // characteristic
  while (ble_get_connection_count() == 0) {
    ;
  }

  while (ble_get_connection_count() != 0) {
    ;
  }

  TEST_ASSERT_TRUE(g_called);
}

void test_ble_destroy(void) {
  BLEStatus status = ble_destroy();
  TEST_ASSERT_EQUAL(BLEStatus::SUCCESS, status);
}

void setup(void) {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_ble_init);
  RUN_TEST(test_ble_send_log_message);
  RUN_TEST(test_ble_sensor_calibration_callback);
  RUN_TEST(test_ble_destroy);
  UNITY_END();
}

void loop(void) { ; }