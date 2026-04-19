#include "Mutex.h"
#include <cassert>

freertos::Mutex::Mutex() : 
    m_mutex({ .handle = xSemaphoreCreateMutexStatic(&m_mutex.buffer) })
{
    assert(m_mutex.handle != NULL);
}

void freertos::Mutex::lock() {
    bool result = xSemaphoreTake(m_mutex.handle, portMAX_DELAY) == pdTRUE;
    assert(result);
}

bool freertos::Mutex::try_lock() {
    return xSemaphoreTake(m_mutex.handle, 0) == pdTRUE;
}

void freertos::Mutex::unlock() {
    bool result = xSemaphoreGive(m_mutex.handle) == pdTRUE;
    assert(result);
}

freertos::Mutex::~Mutex() {
    vSemaphoreDelete(m_mutex.handle);
}