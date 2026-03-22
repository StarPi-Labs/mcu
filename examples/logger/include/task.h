#pragma once

#include <FreeRTOS.h>

// stack size of each task in words
#define TASK_STACK_SIZE 4096

#define DECLARE_STATIC_TASK(handle) \
	void handle (void *data); \
	StaticTask_t _tbuffer_##handle; \
	StackType_t  _tstack_##handle[ TASK_STACK_SIZE ]; \
	TaskHandle_t thandle_##handle;


#define INIT_STATIC_TASK(handle, name, data, priority) \
	thandle_##handle = xTaskCreateStatic(handle, name, TASK_STACK_SIZE, (void*)(data), priority, _tstack_##handle, &_tbuffer_##handle)
