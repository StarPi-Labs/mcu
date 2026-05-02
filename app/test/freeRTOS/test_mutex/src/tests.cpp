#include "tests.h"
#include <unity.h>
#include <Arduino.h>
#include "Mutex.h"
#include <chrono>

using namespace freertos;

void setUp() {}
void tearDown() {}

void test_mutex_basic_lock_unlock() {
    Mutex m;
    m.lock();
    m.unlock();
    TEST_ASSERT_TRUE(true); // If we get here without asserting, it didn't hang
}

void test_mutex_try_lock() {
    Mutex m;
    TEST_ASSERT_TRUE(m.try_lock());
    TEST_ASSERT_FALSE(m.try_lock()); // Not recursive by default
    m.unlock();
    TEST_ASSERT_TRUE(m.try_lock());
    m.unlock();
}

void test_mutex_try_lock_for() {
    Mutex m;
    m.lock();
   
    using namespace std::chrono_literals;

    auto start = std::chrono::steady_clock::now();
    bool result = m.try_lock_for(50ms);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    
    TEST_ASSERT_FALSE(result);
    // Error should be less than 1ms
    TEST_ASSERT_INT64_WITHIN(1, 50, duration.count());
    
    m.unlock();
}

void test_mutex_try_lock_until() {
    Mutex m;
    m.lock();
    
    using namespace std::chrono_literals;

    auto start = std::chrono::steady_clock::now();
    bool result = m.try_lock_until(start + 50ms);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    
    TEST_ASSERT_FALSE(result);
    // Error should be less than 1ms
    TEST_ASSERT_INT64_WITHIN(1, 50, duration.count());
    
    m.unlock();
}

struct TaskParams {
    Mutex& m;
    TaskHandle_t testTask;
};

void helper_task(void* pvParameters) {
    TaskParams& params = *static_cast<TaskParams*>(pvParameters);
    params.m.lock();
    xTaskNotifyGive(params.testTask); // Notify that the lock is acquired
  
    // Wait for the main task to signal us to release the lock
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
    params.m.unlock();
    
    xTaskNotifyGive(params.testTask); // Notify that the task is finished unlocking
    vTaskDelete(NULL);
}

void test_mutex_multithreaded() {
    Mutex m;
    TaskParams params = {m, xTaskGetCurrentTaskHandle()};
    TaskHandle_t helperTaskHandle;
    
    xTaskCreate(helper_task, "helper", configMINIMAL_STACK_SIZE + 1024, &params, 1, &helperTaskHandle);
    
    // Wait for the task to actually acquire the lock via direct task notification
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    
    // Attempting to lock should fail immediately with try_lock
    TEST_ASSERT_FALSE(m.try_lock());
    xTaskNotifyGive(helperTaskHandle); // Signal the helper task to release the lock
    
    // Wait for the task to release it (blocking lock)
    m.lock();
    TEST_ASSERT_TRUE(true); // Proves we were able to acquire the lock after the background task released it
    m.unlock();

    // Wait for the helper task to finish and notify us
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    TEST_ASSERT_TRUE(true); // Ensure the background task ran to completion
}

void runUnityTests() {
    UNITY_BEGIN();
    RUN_TEST(test_mutex_basic_lock_unlock);
    RUN_TEST(test_mutex_try_lock);
    RUN_TEST(test_mutex_try_lock_for);
    RUN_TEST(test_mutex_try_lock_until);
    RUN_TEST(test_mutex_multithreaded);
    UNITY_END();
}