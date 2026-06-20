#include "Logging.h"

namespace mcu::log
{
void init()
{
  using namespace implementation;

#if MCU_LOG_TIMESTAMP_ENABLE && !MCU_LOG_TIMESTAMP_ABSOLUTE
  g_bootTime = std::chrono::steady_clock::now();
#endif

  bool result =
      xTaskCreatePinnedToCore(vTask, "logger", 4096, NULL, tskIDLE_PRIORITY + 2,
                              &g_loggerTaskHandle, 0) == pdPASS;

  assert(result && "Failed to create logger task");
  (void)result;
}

void vTask(void* pvParams)
{
  using namespace implementation;

  while (true) {
    {
      std::unique_lock lock(g_mutex);

      // Wait indefinitely until there is at least one message.
      g_cvBufferEmpty.wait(lock, []() { return g_bufferOffset > 0; });

      // We have a message. Wait up to 100ms to batch more messages,
      // unless the buffer is filling up.
      using namespace std::chrono_literals;

      g_cvBufferEmpty.wait_for(lock, 100ms, []() {
        constexpr std::size_t CAPACITY_THRESHOLD =
            (MCU_LOG_BUFFER_SIZE * 8) / 10; // 80% of MCU_LOG_BUFFER_SIZE
        return g_bufferOffset >= CAPACITY_THRESHOLD;
      });

      // Swap active buffer and release lock
      {
        std::lock_guard inactiveLock(g_inactiveMutex);
        g_activeIndex = 1 - g_activeIndex;
        g_inactiveBufferOffset = g_bufferOffset;
      }

      g_bufferOffset = 0;

      // Clear buffer full bit
      ulTaskNotifyValueClear(NULL, BUFFER_FULL_BIT);
      xTaskNotifyStateClear(NULL);
    }

    // Notify writers that the buffer is now empty
    g_cvBufferFull.notify_all();

    // Get the lock on the inactive buffer (prevent flush() from reading it)
    std::lock_guard lock(g_inactiveMutex);

    // Messages were flushed
    if (g_inactiveBufferOffset == 0)
      continue;

    struct HandlerTask {
      StaticTask_t buffer;
      std::vector<StackType_t> stack;
      TaskHandle_t handle = NULL;
    };

    // Let handlers read the message from the now-inactive buffer and notify
    // when done.
    static std::vector<HandlerTask> handlerTasks(g_targets.size());

    for (std::size_t i = 0; i < g_targets.size(); ++i) {
      handlerTasks[i].stack.resize(g_targets[i].stackSize);

      handlerTasks[i].handle = xTaskCreateStatic(
          [](void* params) {
            std::size_t index = reinterpret_cast<std::size_t>(params);
            std::string_view message(g_buffers[1 - g_activeIndex].data(),
                                     g_inactiveBufferOffset);

            Handler& handler = g_targets[index].handler;
            handler(message);

            // Notify the logger task that this handler is done
            xTaskNotifyGive(g_loggerTaskHandle);

            TaskHandle_t& self = handlerTasks[index].handle;
            self = NULL; // Mark this handler as done
            vTaskDelete(NULL);
          },
          g_targets[i].name, g_targets[i].stackSize, reinterpret_cast<void*>(i),
          g_targets[i].priority, handlerTasks[i].stack.data(),
          &handlerTasks[i].buffer);
    }

    // Wait for all handler tasks to complete.
    std::size_t doneCount = 0;
    bool notified = false;
    while (doneCount < g_targets.size() && !notified) {
      std::uint32_t value;
      xTaskNotifyWait(0, 0xffffffff, &value, portMAX_DELAY);

      if (value & BUFFER_FULL_BIT) {
        // Buffer is full again, break to flush immediately
        notified = true;

        for (auto& task : handlerTasks)
          if (task.handle != NULL)
            vTaskDelete(task.handle);

      } else {
        // A handler or more finished processing
        doneCount += value & ~BUFFER_FULL_BIT;
      }
    }

    g_inactiveBufferOffset = 0;
  }
}

void flushInactive()
{
  using namespace implementation;
  std::lock_guard lock(g_inactiveMutex);

  if (g_inactiveBufferOffset == 0)
    return;

  std::string_view message(g_buffers[1 - g_activeIndex].data(),
                           g_inactiveBufferOffset);

  for (const auto& target : g_targets)
    target.handler(message);

  g_inactiveBufferOffset = 0;
}

void flushActive()
{
  using namespace implementation;
  std::lock_guard lock(g_mutex);

  if (g_bufferOffset == 0)
    return;

  std::string_view message(g_buffers[g_activeIndex].data(), g_bufferOffset);

  for (const auto& target : g_targets)
    target.handler(message);

  g_bufferOffset = 0;

  g_cvBufferFull.notify_all();
}

void flush()
{
  flushInactive();
  flushActive();
}

void addTarget(const char* name, Handler handler, UBaseType_t priority,
               std::uint32_t stackSize)
{
  using namespace implementation;
  // Just reuse g_mutex since shouldn't be called after init()
  std::lock_guard lock(g_mutex);

  g_targets.emplace_back(name, handler, priority, stackSize);
}

} // namespace mcu::log