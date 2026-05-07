#include "tests.h"
#include <Arduino.h>
#include <Logging.h>
#include <unity.h>

static std::string g_capturedOutput;

void setUp() {}

void tearDown()
{
  g_capturedOutput.clear();
}

void customHandler(std::string_view msg)
{
  g_capturedOutput += msg;
}

void test_logging_basic()
{
  mcu_log_info("test {}", 123);
  mcu::flush();

  TEST_ASSERT_EQUAL_STRING("[INFO]: test 123", g_capturedOutput.c_str());
}

void test_logging_multiple_messages()
{
  mcu_log_info("test {}", 1);
  mcu_log_warning("test {}", 2);
  mcu::flush();

  TEST_ASSERT_EQUAL_STRING("[INFO]: test 1[WARNING]: test 2",
                           g_capturedOutput.c_str());
}

void runUnityTests()
{
  mcu::g_logTargets.push_back({customHandler, tskIDLE_PRIORITY + 1});

  UNITY_BEGIN();
  RUN_TEST(test_logging_basic);
  RUN_TEST(test_logging_multiple_messages);
  UNITY_END();
}