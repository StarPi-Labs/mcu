#include <unity.h>

#include <Arduino.h>
#include <Bluetooth.hpp>

void setUp(void) { ; }

void tearDown(void) { ; }

void test_ble_init(void) {
  BLEStatus status = ble_init();
  TEST_ASSERT_EQUAL(BLEStatus::SUCCESS, status);
}

void test_ble_send_log_message(void) {
  LogMessage message;
  message.timestamp = 0x123456789ABCDEF0;
  message.payload_type = P_STRING;
  message.src = S_OTHER;
  message.type = T_SYSLOG;
  message.payload.s = "Test log message";

  BLEStatus status = ble_send_log_message(&message);
  TEST_ASSERT_EQUAL(BLEStatus::SUCCESS, status);
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
  RUN_TEST(test_ble_destroy);
  UNITY_END();
}

void loop(void) { ; }