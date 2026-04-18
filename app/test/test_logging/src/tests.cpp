#include "tests.h"

#include <unity.h>
#include <Logging.h>
#include <string_view>
#include <string>
#include <tuple>

static std::string g_last_log_message;

void setUp() {

}

void tearDown() {
    g_last_log_message.clear();
}

void test_log_handler_simple_message();
void test_log_handler_format_message();

void runUnityTests() {
    mcu::g_logHandlers.push_back([](std::string_view msg) {
        g_last_log_message = std::string(msg);
    });

    UNITY_BEGIN();
    RUN_TEST(test_log_handler_simple_message);
    RUN_TEST(test_log_handler_format_message);
    UNITY_END();
}

void test_log_handler_simple_message() {
    constexpr std::string_view TEST_STRING = " test: To the moon!";
    std::string expected = std::string(MCU_LOG_PREFIX_INFO) + std::string(TEST_STRING);
    
    mcu_log_info(TEST_STRING);
    mcu::flush();

    TEST_ASSERT_EQUAL_STRING(expected.c_str(), g_last_log_message.c_str());
}

void test_log_handler_format_message() {
    constexpr std::tuple args{"World", 365, 3.14f};
    constexpr std::string_view TEST_STRING = "test: Hello {}, there are {} days in a year and pi is {}";
    std::string expected = std::string(MCU_LOG_PREFIX_INFO) +
        std::apply([=](auto&&... unpacked) { 
            return std::format(TEST_STRING, unpacked...); 
        }, args);

    std::apply([=](auto&&... unpacked) { 
        mcu_log_info(TEST_STRING, unpacked...); 
    }, args);
    mcu::flush();

    TEST_ASSERT_EQUAL_STRING(expected.c_str(), g_last_log_message.c_str());
}