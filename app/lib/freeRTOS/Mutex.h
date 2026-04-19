#pragma once

#include <FreeRTOS.h>
#include <chrono>
#include <array>

namespace freertos {
/**
 * @brief Wrapper per mutex FreeRTOS
 * Implementa: BasicLockable, Lockable, TimedLockable
 */
class Mutex {
public:
    Mutex();
    /**
     * @brief Acquisisce il mutex, bloccando il thread finché non è disponibile.
     */
    void lock();

    /** 
     * @brief Prova ad acquisire il mutex senza bloccare.
     * @return true se il mutex è stato acquisito, false altrimenti.
     */
    bool try_lock();

    /** 
     * @brief Prova ad acquisire il mutex per un tempo specifico.
     * @param relativeTime il tempo di attesa.
     * @return true se il mutex è stato acquisito, false altrimenti.
     */
    template <typename Rep, typename Period = std::ratio<1>>
    bool try_lock_for(const std::chrono::duration<Rep, Period>& relativeTime) {
        auto ticks = std::chrono::duration_cast<std::chrono::milliseconds>(relativeTime).count() / portTICK_PERIOD_MS;
        return xSemaphoreTake(m_handle, ticks) == pdTRUE;
    }
   
    /** 
     * @brief Prova ad acquisire il mutex fino a un tempo specifico.
     * @param absoluteTime il tempo assoluto di scadenza.
     * @return true se il mutex è stato acquisito, false altrimenti.
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
     * @brief Rilascia il mutex.
     */
    void unlock();

    ~Mutex();
private:
    StaticSemaphore_t m_buffer;
    SemaphoreHandle_t m_handle;
};
} // namespace freertos