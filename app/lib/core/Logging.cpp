#include "Logging.h"

namespace mcu::log
{
void init()
{
  assert(Serial && "Serial port not initialized");
  g_targets.push_back(ARDUINO_SERIAL);

#if MCU_LOG_TIMESTAMP_ENABLE
  g_bootTime = std::chrono::steady_clock::now();
#endif

  bool result =
      xTaskCreatePinnedToCore(vTask, "logger", 4096, NULL, tskIDLE_PRIORITY + 2,
                              NULL, 0) == pdPASS;
  assert(result && "Failed to create logger task");
  (void)result;
}

void vTask(void* pvParams)
{
  using namespace implementation;

  const static TaskHandle_t loggerTaskHandle = xTaskGetCurrentTaskHandle();

  while (true) {
    static std::size_t messageLength;

    {
      std::unique_lock lock(g_mutex);

      // 1. Wait indefinitely until there is at least one message.
      g_cvBufferEmpty.wait(lock, []() { return g_bufferOffset > 0; });

      // 2. We have a message. Wait up to 100ms to batch more messages,
      // unless the buffer is filling up.
      using namespace std::chrono_literals;

      g_cvBufferEmpty.wait_for(lock, 100ms, []() {
        constexpr std::size_t CAPACITY_THRESHOLD =
            (MCU_LOG_BUFFER_SIZE * 8) / 10; // 80% of MCU_LOG_BUFFER_SIZE
        return g_bufferOffset >= CAPACITY_THRESHOLD;
      });

      // 3. Swap active buffer and release lock
      g_activeIndex = 1 - g_activeIndex;
      messageLength = g_bufferOffset;
      g_bufferOffset = 0;
    }

    // Notify writers that the buffer is now empty
    g_cvBufferFull.notify_all();

    // Buffer was flushed
    if (messageLength == 0)
      continue;

    // 4. Let handlers read the message from the now-inactive buffer and notify
    // when done.
    for (std::size_t i = 0; i < g_targets.size(); ++i)
      xTaskCreate(
          [](void* params) {
            Handler& handler = *(static_cast<Handler*>(params));

            std::string_view message(g_buffers[1 - g_activeIndex].data(),
                                     messageLength);
            handler(message);
            xTaskNotifyGive(loggerTaskHandle);
            vTaskDelete(NULL);
          },
          "log_target", 4096, static_cast<void*>(&g_targets[i].handler),
          g_targets[i].priority, NULL);

    // TODO: use task notification as event groups to immediately
    // free and swap buffers if the main queue is full:
    // PREFER RECENT MESSAGES OVER OLD
    // 5. Wait for all handler tasks to complete.
    for (std::size_t i = 0; i < g_targets.size(); ++i)
      ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
  }
}

void flush()
{
  using namespace implementation;

  {
    std::lock_guard lock(g_mutex);

    if (g_bufferOffset == 0)
      return;

    std::string_view message(g_buffers[g_activeIndex].data(), g_bufferOffset);

    for (const auto& target : g_targets)
      target.handler(message);

    g_bufferOffset = 0;
  }

  g_cvBufferFull.notify_all();
}

} // namespace mcu::log