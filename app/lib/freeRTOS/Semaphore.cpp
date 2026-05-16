#include "Semaphore.h"

// Use xSemaphoreCreateBinaryStatic for binary semaphores
template <>
freertos::CountingSemaphore<1>::CountingSemaphore(UBaseType_t initialCount) :
    m_semaphore({ .handle = xSemaphoreCreateBinaryStatic(&m_semaphore.buffer) })
{
    configASSERT(m_semaphore.handle != NULL && "Failed to create binary semaphore");

    if (initialCount != 0) {
        xSemaphoreGive(m_semaphore.handle);
    }
}