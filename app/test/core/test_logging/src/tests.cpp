#include "tests.h"
#include <Arduino.h>
// Note: the timestamp is disabled for testing to make it easier to assert log
// output.
#include <Logging.h>
#include <unity.h>

static std::string g_capturedOutput;

void setUp() {}

void tearDown()
{
  g_capturedOutput.clear();
}

void test_logging_basic()
{
  mcu_log_info("test", "test {}", 123);
  mcu::log::flush();

  std::string expectedOutput =
      std::format("test.{} {}[{}]: test 123\n", mcu::log::INFO_STRING,
                  pcTaskGetName(NULL), uxTaskGetTaskNumber(NULL));

  TEST_ASSERT_EQUAL_STRING(expectedOutput.c_str(), g_capturedOutput.c_str());
}

void test_logging_multiple_messages()
{
  mcu_log_info("test", "test {}", 1);
  mcu_log_warning("test", "test {}", 2);
  mcu::log::flush();

  std::string expectedOutput = std::format(
      "test.{} {}[{}]: test 1\ntest.{} {}[{}]: test 2\n", mcu::log::INFO_STRING,
      pcTaskGetName(NULL), uxTaskGetTaskNumber(NULL), mcu::log::WARNING_STRING,
      pcTaskGetName(NULL), uxTaskGetTaskNumber(NULL));

  TEST_ASSERT_EQUAL_STRING(expectedOutput.c_str(), g_capturedOutput.c_str());
}

void runUnityTests()
{
  mcu::log::addTarget(
      "Custom", [](std::string_view message) { g_capturedOutput += message; },
      tskIDLE_PRIORITY + 1, 2048);

  UNITY_BEGIN();
  RUN_TEST(test_logging_basic);
  RUN_TEST(test_logging_multiple_messages);
  UNITY_END();
}