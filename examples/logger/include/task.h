#pragma once

#include "FreeRTOSConfig.h"
#include <Arduino.h>


// Stack size of each task in words, with zero extra stack this results in 1k
// words of usable stack
#define TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + configIDLE_TASK_STACK_SIZE + 2048)


// TaskDescriptor_t is a struct that contains all the information about a task,
// This allows us to create static tasks without using dynamic memory allocation.
// The task function should take a pointer to the TaskDescriptor_t struct as an
// argument, which allows the task to access its own metadata and stack memory.
// The last_wake field is used for the TASK_WAIT_HZ macro to keep track of when
// the task was last woken up.
typedef struct _TaskDescriptor_t {
	void (*func)(_TaskDescriptor_t*); // task function
	StaticTask_t buffer; // task matadata
	TaskHandle_t handle; // task handle
	TickType_t last_wake; // last wake time in ticks
	BaseType_t was_delayed; // set to true if the task was delayed last run
	StackType_t stack[TASK_STACK_SIZE]; // task stack memory
} TaskDescriptor_t;


// type checking
#ifdef __cplusplus
  #define _IS_TASKDESCRIPTOR_POINTER(x) (std::is_same<decltype(x), TaskDescriptor_t*>::value)
#else
  #include <stdbool.h>
  #define _IS_TASKDESCRIPTOR_POINTER(x) _Generic((x), TaskDescriptor_t*: true, default: false)
#endif


// Declare a static task, this creates a TaskDescriptor_t struct with the appropriate
// function pointer and stack memory.
// This does not create the task, you must call INIT_STATIC_TASK to do that
#define DECLARE_STATIC_TASK(symbol) \
	void symbol (TaskDescriptor_t *data); \
	TaskDescriptor_t symbol##_descriptor = { \
		.func = symbol, \
	}


// Initialize a static task, this creates the task using xTaskCreateStatic and
// stores the handle in the TaskDescriptor_t struct. The task will be pinned
// to the specified core.
#define INIT_STATIC_TASK(symbol, name, data, priority, core) do { \
	symbol##_descriptor.handle = xTaskCreateStaticPinnedToCore( \
			(TaskFunction_t)symbol##_descriptor.func, \
			name, \
			TASK_STACK_SIZE, \
			(void*)(&symbol##_descriptor), \
			priority, \
			symbol##_descriptor.stack, \
			&(symbol##_descriptor.buffer), \
			core \
		); \
	} while (0)


#define TASK_IS_INITIALIZED(symbol) (symbol##_descriptor.handle != NULL)


// Since tasks should never return, we can use the noreturn attribute to catch
// bugs where a task accidentally returns. This will cause a compile error if a
// task function returns.
#define TASK __attribute__((noreturn)) void


// FIXME: use a different logger
// TODO: print the task name
// Wait until the next period, this should be called at the end of each task loop
#define TASK_WAIT_HZ(desc, freq) do { \
		static_assert(_IS_TASKDESCRIPTOR_POINTER(desc), "First argument must be of type TaskDescriptor_t"); \
		desc->was_delayed = xTaskDelayUntil(&(desc->last_wake), pdMS_TO_TICKS(1000/freq)); \
		if (desc->was_delayed == false) { \
			Serial.printf("[%s:%d]: task failed to meet deadline\n", __FILE__, __LINE__); \
		} \
	} while (0)
