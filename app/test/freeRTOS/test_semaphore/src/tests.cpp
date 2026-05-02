#include "tests.h"

#include <Arduino.h>
#include <chrono>
#include <unity.h>

#include "Semaphore.h"

using namespace freertos;

void setUp() {}

void tearDown() {}

void test_counting_semaphore_default_initial_count() {
    CountingSemaphore<2> semaphore;

    TEST_ASSERT_TRUE(semaphore.try_acquire());
    TEST_ASSERT_TRUE(semaphore.try_acquire());
    TEST_ASSERT_FALSE(semaphore.try_acquire());
}

void test_counting_semaphore_release_and_acquire() {
    CountingSemaphore<2> semaphore(0);

    TEST_ASSERT_FALSE(semaphore.try_acquire());
    semaphore.release();
    TEST_ASSERT_TRUE(semaphore.try_acquire());
}

void test_binary_semaphore_try_acquire_for() {
    BinarySemaphore semaphore(0);

    using namespace std::chrono_literals;

    auto start = std::chrono::steady_clock::now();
    bool result = semaphore.try_acquire_for(50ms);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_INT64_WITHIN(1, 50, duration.count());
}

void test_binary_semaphore_try_acquire_until() {
    BinarySemaphore semaphore(0);

    using namespace std::chrono_literals;

    auto start = std::chrono::steady_clock::now();
    bool result = semaphore.try_acquire_until(start + 50ms);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_INT64_WITHIN(1, 50, duration.count());
}

struct TaskParams {
    BinarySemaphore& semaphore;
    TaskHandle_t testTask;
};

void helper_task(void* pvParameters) {
    auto& params = *static_cast<TaskParams*>(pvParameters);

    params.semaphore.acquire();
    xTaskNotifyGive(params.testTask);

    vTaskDelete(NULL);
}

void test_binary_semaphore_multithreaded() {
    BinarySemaphore semaphore(0);
    TaskParams params = {semaphore, xTaskGetCurrentTaskHandle()};
    TaskHandle_t helperTaskHandle;

    xTaskCreate(helper_task,
                "sem_helper",
                configMINIMAL_STACK_SIZE + 1024,
                &params,
                1,
                &helperTaskHandle);

    TEST_ASSERT_EQUAL_UINT32(0, ulTaskNotifyTake(pdFALSE, 10 / portTICK_PERIOD_MS));

    semaphore.release();
    TEST_ASSERT_EQUAL_UINT32(1, ulTaskNotifyTake(pdFALSE, portMAX_DELAY));
}

void test_binary_semaphore_max() {
    TEST_ASSERT_EQUAL_UINT32(1, BinarySemaphore::max());
}

void runUnityTests() {
    UNITY_BEGIN();
    RUN_TEST(test_counting_semaphore_default_initial_count);
    RUN_TEST(test_counting_semaphore_release_and_acquire);
    RUN_TEST(test_binary_semaphore_try_acquire_for);
    RUN_TEST(test_binary_semaphore_try_acquire_until);
    RUN_TEST(test_binary_semaphore_multithreaded);
    RUN_TEST(test_binary_semaphore_max);
    UNITY_END();
}