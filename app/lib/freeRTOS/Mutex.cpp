#include "Mutex.h"
#include <cassert>

mcu::Mutex::Mutex() : 
    m_handle(xSemaphoreCreateMutexStatic(&m_buffer))
{
    assert(m_handle != NULL);
}

void mcu::Mutex::lock() {
    bool result = xSemaphoreTake(m_handle, portMAX_DELAY) == pdTRUE;
    assert(result);
}

bool mcu::Mutex::try_lock() {
    return xSemaphoreTake(m_handle, 0) == pdTRUE;
}

void mcu::Mutex::unlock() {
    bool result = xSemaphoreGive(m_handle) == pdTRUE;
    assert(result);
}

mcu::Mutex::~Mutex() {
    vSemaphoreDelete(m_handle);
}