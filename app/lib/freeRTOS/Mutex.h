#pragma once

#include <FreeRTOS.h>
#include <chrono>
#include <array>
#include "Semaphore.h"

namespace freertos {
/**
 * @brief Wrapper for FreeRTOS mutex
 * Implements: BasicLockable, Lockable, TimedLockable
 */
class Mutex {
public:
    Mutex();
    ~Mutex();

    /**
     * @brief Acquires the mutex, blocking the thread until it is available.
     */
    void lock();

    /** 
     * @brief Attempts to acquire the mutex without blocking.
     * @return true if the mutex was acquired, false otherwise.
     */
    bool try_lock();

    /** 
     * @brief Attempts to acquire the mutex for a specific time.
     * @param relativeTime the wait time.
     * @return true if the mutex was acquired, false otherwise.
     */
    template <typename Rep, typename Period = std::ratio<1>>
    bool try_lock_for(const std::chrono::duration<Rep, Period>& relativeTime) {
        auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>(relativeTime).count() / portTICK_PERIOD_MS;
        // Round up (+1) to ensure we wait at least as long as requested (to avoid truncation)
        return xSemaphoreTake(m_mutex.handle, ticks + 1) == pdTRUE;
    }
   
    /** 
     * @brief Attempts to acquire the mutex until a specific time.
     * @param absoluteTime the absolute expiration time.
     * @return true if the mutex was acquired, false otherwise.
     */
    template <typename Clock, typename Duration = typename Clock::duration>
    bool try_lock_until(const std::chrono::time_point<Clock, Duration>& absoluteTime) {
        auto now = Clock::now();

        if (absoluteTime <= now) 
            return try_lock();

        auto relativeTime = absoluteTime - now;
        return try_lock_for(relativeTime);
    }

    /** 
     * @brief Releases the mutex.
     */
    void unlock();
private:
    NativeSemaphore_t m_mutex;
};
} // namespace freertos