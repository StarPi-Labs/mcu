#pragma once

#include <FreeRTOS.h>

namespace freertos {
struct NativeSemaphore_t {
    StaticSemaphore_t buffer;
    SemaphoreHandle_t handle;
};
}