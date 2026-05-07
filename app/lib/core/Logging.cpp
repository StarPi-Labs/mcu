#include "Logging.h"

namespace mcu
{
void vTaskLogger(void* pvParams)
{
  using namespace implementation;

  static TaskHandle_t loggerTaskHandle = xTaskGetCurrentTaskHandle();

  while (true) {
    static std::size_t messageLength;

    {
      std::unique_lock lock(g_logMutex);

      // 1. Wait indefinitely until there is at least one message.
      g_cvLogEmpty.wait(lock, []() { return g_bufferOffset > 0; });

      // 2. We have a message. Wait up to 100ms to batch more messages,
      // unless the buffer is filling up.
      using namespace std::chrono_literals;

      g_cvLogEmpty.wait_for(lock, 100ms, []() {
        constexpr std::size_t CAPACITY_THRESHOLD =
            (MCU_LOG_BUFFER_SIZE * 8) / 10; // 80% of MCU_LOG_BUFFER_SIZE
        return g_bufferOffset >= CAPACITY_THRESHOLD;
      });

      // 3. Swap active buffer and release lock
      g_logActiveIndex = 1 - g_logActiveIndex;
      messageLength = g_bufferOffset;
      g_bufferOffset = 0;
    }

    // Notify writers that the buffer is now empty
    g_cvLogFull.notify_all();

    // Buffer was flushed
    if (messageLength == 0)
      continue;

    // 4. Let handlers read the message from the now-inactive buffer and notify
    // when done.
    for (std::size_t i = 0; i < g_logTargets.size(); ++i)
      xTaskCreate(
          [](void* params) {
            LogTargets::LogHandler& handler =
                *(static_cast<LogTargets::LogHandler*>(params));

            std::string_view message(g_logBuffers[1 - g_logActiveIndex].data(),
                                     messageLength);
            handler(message);
            xTaskNotifyGive(loggerTaskHandle);
            vTaskDelete(NULL);
          },
          "log_target", 2048, static_cast<void*>(&g_logTargets[i].handler),
          g_logTargets[i].priority, NULL);

    // 5. Wait for all handler tasks to complete.
    for (std::size_t i = 0; i < g_logTargets.size(); ++i)
      ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
  }
}

void flush()
{
  using namespace implementation;

  {
    std::lock_guard lock(g_logMutex);

    if (g_bufferOffset == 0)
      return;

    std::string_view message(g_logBuffers[g_logActiveIndex].data(),
                             g_bufferOffset);

    for (const auto& target : g_logTargets)
      target.handler(message);

    g_bufferOffset = 0;
  }

  g_cvLogFull.notify_all();
}

} // namespace mcu