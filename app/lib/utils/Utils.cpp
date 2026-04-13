#include "Utils.h"
#include <cassert>

mcu::Mutex::Mutex() : 
    m_mutex(xSemaphoreCreateMutexStatic(&m_mutexBuffer))
{
    assert(m_mutex != NULL);
}

void mcu::Mutex::lock() {
    bool result = xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE;
    assert(result);
}

bool mcu::Mutex::try_lock() {
    return xSemaphoreTake(m_mutex, 0) == pdTRUE;
}

void mcu::Mutex::unlock() {
    bool result = xSemaphoreGive(m_mutex) == pdTRUE;
    assert(result);
}

mcu::Mutex::~Mutex() {
    vSemaphoreDelete(m_mutex);
}