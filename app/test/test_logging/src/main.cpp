#include <unity.h>
#include <logging.h>
#include <iostream>

void setUp(void) {
    // Non fa niente
}

void tearDown(void) {
    // Non fa niente
}

void test_log_buffer_empty_at_start(void);
void test_log_buffer_message_read_successful(void);

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_log_buffer_empty_at_start);
    RUN_TEST(test_log_buffer_message_read_successful);
    return UNITY_END();
}

void test_log_buffer_empty_at_start(void) {
    TEST_ASSERT_TRUE(mcu::g_logBuffer.length == 0);
}

#define TEST_LOG_MESSAGE_SIMPLE "Hello World!"

void test_log_buffer_message_read_successful(void) {
    
    mcu_log_info(TEST_LOG_MESSAGE_SIMPLE);

    TEST_ASSERT_EQUAL_STRING(MCU_LOG_PREFIX_INFO TEST_LOG_MESSAGE_SIMPLE,
        mcu::g_logBuffer.data.data());
}