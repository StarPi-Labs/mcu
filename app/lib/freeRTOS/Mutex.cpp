#include "Mutex.h"

freertos::Mutex::Mutex() : 
    m_mutex({ .handle = xSemaphoreCreateMutexStatic(&m_mutex.buffer) })
{
    configASSERT(m_mutex.handle != NULL && "Failed to create mutex");
}

void freertos::Mutex::lock() {
    bool result = xSemaphoreTake(m_mutex.handle, portMAX_DELAY) == pdTRUE;
    configASSERT(result && "Failed to take mutex");
    (void)result;
}

bool freertos::Mutex::try_lock() {
    return xSemaphoreTake(m_mutex.handle, 0) == pdTRUE;
}

void freertos::Mutex::unlock() {
    bool result = xSemaphoreGive(m_mutex.handle) == pdTRUE;
    configASSERT(result && "Failed to give mutex");
    (void)result;
}

freertos::Mutex::~Mutex() {
    vSemaphoreDelete(m_mutex.handle);
}