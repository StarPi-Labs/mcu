#include "Logging.h"

void mcu::vTaskLogger(void *pvParams) {
  while (true) {
    {
      std::unique_lock lock(g_logBuffer.mutex);

      // 1. Aspetta indefinitamente finché non c'è almeno un messaggio
      g_logBuffer.cvBufferEmpty.wait(lock,
                                     [&]() { return g_logBuffer.length > 0; });

      // 2. Abbiamo un messaggio. Aspettiamo fino a 100ms per raggruppare altri
      // messaggi, a meno che il buffer non si stia riempiendo (es. raggiunge
      // l'80% della capacità)
      using namespace std::chrono_literals;

      g_logBuffer.cvBufferEmpty.wait_for(lock, 100ms, [&]() {
        constexpr std::size_t CAPACITY_THRESHOLD =
            (MCU_LOG_BUFFER_SIZE * 8) / 10;
        return g_logBuffer.length >= CAPACITY_THRESHOLD;
      });

      // Lascia gli handler leggere

      struct HandlerParam {
        std::size_t index;
        TaskHandle_t loggerTask;
      };

      std::vector<HandlerParam> params(g_logHandlers.size());
      std::vector<TaskHandle_t> handlerTasks(g_logHandlers.size());
      TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();

      for (std::size_t i = 0; i < g_logHandlers.size(); ++i) {
        params[i] = {i, currentTask};

        bool result =
            xTaskCreate(
                [](void *param) -> void {
                  HandlerParam *hp = reinterpret_cast<HandlerParam *>(param);

                  g_logHandlers[hp->index](std::string_view(
                      g_logBuffer.data.data(), g_logBuffer.length));

                  xTaskNotifyGive(hp->loggerTask);
                  vTaskDelete(nullptr);
                },
                "LogHandler", configMINIMAL_STACK_SIZE,
                reinterpret_cast<void *>(&params[i]), tskIDLE_PRIORITY + 1,
                &handlerTasks[i]) == pdTRUE;

        assert(result);
      }

      // Attesa termine di tutti i task
      for (std::size_t i = 0; i < g_logHandlers.size(); ++i)
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

      g_logBuffer.length = 0;
    }

    g_logBuffer.cvBufferFull.notify_all();
  }
}

void mcu::flush() {
  {
    std::lock_guard lock(g_logBuffer.mutex);

    if (g_logBuffer.length == 0)
      return;

    for (const auto &handler : g_logHandlers)
      handler(std::string_view(g_logBuffer.data.data(), g_logBuffer.length));

    g_logBuffer.length = 0;
  }
  
  g_logBuffer.cvBufferFull.notify_all();
}
