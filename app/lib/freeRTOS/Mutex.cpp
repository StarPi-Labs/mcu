#include "Mutex.h"
#include <cassert>

freertos::Mutex::Mutex() : 
    m_handle(xSemaphoreCreateMutexStatic(&m_buffer))
{
    assert(m_handle != NULL);
}

void freertos::Mutex::lock() {
    bool result = xSemaphoreTake(m_handle, portMAX_DELAY) == pdTRUE;
    assert(result);
}

bool freertos::Mutex::try_lock() {
    return xSemaphoreTake(m_handle, 0) == pdTRUE;
}

void freertos::Mutex::unlock() {
    bool result = xSemaphoreGive(m_handle) == pdTRUE;
    assert(result);
}

freertos::Mutex::~Mutex() {
    vSemaphoreDelete(m_handle);
}