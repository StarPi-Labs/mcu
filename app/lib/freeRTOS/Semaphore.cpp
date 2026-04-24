#include "Semaphore.h"

// Use xSemaphoreCreateBinaryStatic for binary semaphores
template <>
freertos::BinarySemaphore::CountingSemaphore(UBaseType_t initialCount) :
    m_semaphore({ .handle = xSemaphoreCreateBinaryStatic(&m_semaphore.buffer) })
{
    configASSERT(m_semaphore.handle != NULL && "Failed to create binary semaphore");
}