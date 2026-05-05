#include "tests.h"

#include <Arduino.h>
#include <chrono>
#include <condition_variable>
#include <queue.h>
#include <unity.h>

#include "ConditionVariable.h"
#include "Mutex.h"

using namespace freertos;

void setUp() {}

void tearDown() {}

void test_condition_variable_wait_for_times_out()
{
  ConditionVariable cv;
  Mutex mutex;

  using namespace std::chrono_literals;

  mutex.lock();
  auto start = std::chrono::steady_clock::now();
  std::cv_status status = cv.wait_for(mutex, 50ms);
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start);
  mutex.unlock();

  TEST_ASSERT_EQUAL(std::cv_status::timeout, status);
  TEST_ASSERT_INT64_WITHIN(10, 50, elapsed.count());
}

struct WaitOneTaskParams {
  ConditionVariable cv;
  Mutex mutex;
  volatile bool ready = false;
  TaskHandle_t testTask = xTaskGetCurrentTaskHandle();
};

void wait_one_task(void* pvParameters)
{
  auto& params = *static_cast<WaitOneTaskParams*>(pvParameters);

  params.mutex.lock();
  params.cv.wait(params.mutex, [&params] { return params.ready; });
  params.mutex.unlock();

  xTaskNotifyGive(params.testTask);
  vTaskDelete(NULL);
}

void test_condition_variable_notify_one_wakes_waiter()
{
  WaitOneTaskParams params;
  TaskHandle_t waiterTask = nullptr;

  xTaskCreate(wait_one_task, "cv_wait_one", configMINIMAL_STACK_SIZE + 1024,
              &params, 1, &waiterTask);

  vTaskDelay(10 / portTICK_PERIOD_MS);

  // Note: locking is not nedeed (this the only task that can modify ready).
  params.ready = true;
  params.cv.notify_one();

  TEST_ASSERT_EQUAL_UINT32(1,
                           ulTaskNotifyTake(pdTRUE, 200 / portTICK_PERIOD_MS));
}

using WaitAllTaskParams = WaitOneTaskParams;

void wait_all_task(void* pvParameters)
{
  auto& params = *static_cast<WaitAllTaskParams*>(pvParameters);

  params.mutex.lock();
  params.cv.wait(params.mutex, [&params] { return params.ready; });
  params.mutex.unlock();

  xTaskNotifyGive(params.testTask);
  vTaskDelete(NULL);
}

void test_condition_variable_notify_all_wakes_all_waiters()
{
  WaitAllTaskParams params;
  TaskHandle_t waiterTaskA = nullptr;
  TaskHandle_t waiterTaskB = nullptr;

  xTaskCreate(wait_all_task, "cv_wait_all_a", configMINIMAL_STACK_SIZE + 1024,
              &params, 1, &waiterTaskA);
  xTaskCreate(wait_all_task, "cv_wait_all_b", configMINIMAL_STACK_SIZE + 1024,
              &params, 1, &waiterTaskB);

  vTaskDelay(10 / portTICK_PERIOD_MS);

  params.ready = true;
  params.cv.notify_all();

  uint32_t wakeCount = ulTaskNotifyTake(pdTRUE, 200 / portTICK_PERIOD_MS);
  if (wakeCount == 2) {
    TEST_ASSERT_EQUAL_UINT32(2, wakeCount);
  } else {
    TEST_ASSERT_EQUAL_UINT32(1, wakeCount);
    TEST_ASSERT_EQUAL_UINT32(
        1, ulTaskNotifyTake(pdTRUE, 200 / portTICK_PERIOD_MS));
  }
}

struct PriorityWaitTaskParams {
  PriorityConditionVariable cv;
  Mutex mutex;
  volatile bool ready = false;
  TaskHandle_t testTask = xTaskGetCurrentTaskHandle();
};

void priority_wait_task(void* pvParameters)
{
  auto& params = *static_cast<PriorityWaitTaskParams*>(pvParameters);

  params.mutex.lock();
  params.cv.wait(params.mutex, [&params] { return params.ready; });
  params.mutex.unlock();

  xTaskNotify(params.testTask, uxTaskPriorityGet(NULL), eSetValueWithOverwrite);
  vTaskDelete(NULL);
}

void test_priority_condition_variable_wakes_highest_priority_first()
{
  PriorityWaitTaskParams params;
  TaskHandle_t waiterTaskLow = nullptr;
  TaskHandle_t waiterTaskHigh = nullptr;

  xTaskCreate(priority_wait_task, "cv_wait_low",
              configMINIMAL_STACK_SIZE + 1024, &params, 1, &waiterTaskLow);

  vTaskDelay(10 / portTICK_PERIOD_MS);

  xTaskCreate(priority_wait_task, "cv_wait_high",
              configMINIMAL_STACK_SIZE + 1024, &params, 2, &waiterTaskHigh);

  vTaskDelay(10 / portTICK_PERIOD_MS);

  params.ready = true;
  params.cv.notify_one();

  uint32_t wokenPriority = 0;
  xTaskNotifyWait(0x00, 0xFFFFFFFF, &wokenPriority, 200 / portTICK_PERIOD_MS);

  params.cv.notify_one();
  xTaskNotifyWait(0x00, 0xFFFFFFFF, nullptr, 200 / portTICK_PERIOD_MS);

  TEST_ASSERT_EQUAL_UINT32(2, wokenPriority);
}

void runUnityTests()
{
  UNITY_BEGIN();
  RUN_TEST(test_condition_variable_wait_for_times_out);
  RUN_TEST(test_condition_variable_notify_one_wakes_waiter);
  RUN_TEST(test_condition_variable_notify_all_wakes_all_waiters);
  RUN_TEST(test_priority_condition_variable_wakes_highest_priority_first);
  UNITY_END();
}
