#pragma once

#include <FreeRTOS.h>
#include <chrono>

namespace freertos {

/**
 * @brief Native FreeRTOS semaphore storage and handle wrapper.
 */
struct NativeSemaphore_t {
    StaticSemaphore_t buffer;
    SemaphoreHandle_t handle;
};

/**
 * @brief Fixed-capacity counting semaphore backed by FreeRTOS.
 *
 * @tparam maxCount Maximum number of tokens the semaphore can hold.
 *
 * The class exposes a standard semaphore API:
 * - @ref acquire blocks until a token is available.
 * - @ref try_acquire attempts to take a token without blocking.
 * - @ref try_acquire_for and @ref try_acquire_until provide timed waits.
 * - @ref release returns a token to the semaphore.
 */
template <UBaseType_t maxCount>
class CountingSemaphore {
public:
    /**
     * @brief Creates the semaphore with an optional initial token count.
     *
     * @param initialCount Initial number of available tokens.
     *                     Defaults to @p maxCount.
     */
    CountingSemaphore(UBaseType_t initialCount = maxCount);

    /**
     * @brief Destroys the underlying FreeRTOS semaphore.
     */
    ~CountingSemaphore();

    /**
     * @brief Adds one token to the semaphore.
     */
    void release();

    /**
     * @brief Blocks until a token becomes available.
     */
    void acquire();

    /**
     * @brief Attempts to acquire a token without blocking.
     *
     * @return true if a token was acquired, false otherwise.
     */
    bool try_acquire();

    /**
     * @brief Attempts to acquire a token for a relative duration.
     *
     * @param relativeTime Maximum time to wait.
     * @return true if a token was acquired, false otherwise.
     */
    template <typename Rep, typename Period>
    bool try_acquire_for(const std::chrono::duration<Rep, Period>& relativeTime) {
        auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>(relativeTime).count() / portTICK_PERIOD_MS;
        return xSemaphoreTake(m_semaphore.handle, ticks) == pdTRUE;
    }

    /**
     * @brief Attempts to acquire a token until an absolute time point.
     *
     * @param absoluteTime Deadline to wait for.
     * @return true if a token was acquired, false otherwise.
     */
    template <typename Clock, typename Duration = typename Clock::duration>
    bool try_acquire_until(const std::chrono::time_point<Clock, Duration>& absoluteTime) {
        auto now = std::chrono::steady_clock::now();

        if (absoluteTime <= now)
            return try_acquire();

        auto relativeTime = absoluteTime - now;
        return try_acquire_for(relativeTime);
    }

    /**
     * @brief Returns the maximum capacity of the semaphore.
     */
    static constexpr UBaseType_t max() {
        return maxCount;
    }

private:
    NativeSemaphore_t m_semaphore;
};

/**
 * @brief Binary semaphore alias.
 */
using BinarySemaphore = CountingSemaphore<1>;

template <UBaseType_t maxCount>
CountingSemaphore<maxCount>::CountingSemaphore(UBaseType_t initialCount)
    : m_semaphore({ .handle = xSemaphoreCreateCountingStatic(maxCount, initialCount, &m_semaphore.buffer) })
{
    configASSERT(initialCount <= maxCount);
    configASSERT(m_semaphore.handle != nullptr && "Failed to create counting semaphore");
}

template <UBaseType_t maxCount>
CountingSemaphore<maxCount>::~CountingSemaphore() {
    vSemaphoreDelete(m_semaphore.handle);
}

template <UBaseType_t maxCount>
void CountingSemaphore<maxCount>::release() {
    bool result = xSemaphoreGive(m_semaphore.handle) == pdTRUE;
    configASSERT(result && "Failed to release counting semaphore");
    (void)result;
}

template <UBaseType_t maxCount>
void CountingSemaphore<maxCount>::acquire() {
    bool result = xSemaphoreTake(m_semaphore.handle, portMAX_DELAY) == pdTRUE;
    configASSERT(result && "Failed to acquire counting semaphore");
    (void)result;
}

template <UBaseType_t maxCount>
bool CountingSemaphore<maxCount>::try_acquire() {
    return xSemaphoreTake(m_semaphore.handle, 0) == pdTRUE;
}
} // namespace freertos