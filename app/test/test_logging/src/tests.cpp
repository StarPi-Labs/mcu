#include "tests.h"

#include <unity.h>
#include <Logging.h>
#include <string_view>

void runUnityTests() {
    UNITY_BEGIN();
    RUN_TEST(test_log_buffer_empty_at_start);
    RUN_TEST(test_log_buffer_simple_message);
    RUN_TEST(test_log_buffer_format_message);
    UNITY_END();
}

void test_log_buffer_empty_at_start() {
    TEST_ASSERT_TRUE(mcu::g_logBuffer.length == 0);
}

void test_log_buffer_simple_message() {
    constexpr std::string_view message = "Hello World!";
    mcu_log_info("{}", message);

    constexpr std::string_view expected = MCU_LOG_PREFIX_INFO "Hello World!";
    const std::string_view actual(mcu::g_logBuffer.data.data(), mcu::g_logBuffer.length);

    TEST_ASSERT_TRUE(expected == actual);
    mcu::g_logBuffer.length = 0;
}

void test_log_buffer_format_message() {
    constexpr std::string_view name = "World";
    mcu_log_info("Hello {}!", name);

    constexpr std::string_view expected = MCU_LOG_PREFIX_INFO "Hello World!";
    const std::string_view actual(mcu::g_logBuffer.data.data(), mcu::g_logBuffer.length);

    TEST_ASSERT_TRUE(expected == actual);
    mcu::g_logBuffer.length = 0;
}