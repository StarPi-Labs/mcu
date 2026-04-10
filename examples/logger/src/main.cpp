#include <Arduino.h>
#include <FreeRTOS.h>

#include "logger.h"
#include "task.h"

DECLARE_STATIC_TASK(task1);
DECLARE_STATIC_TASK(task2);

void setup(void)
{
	message_queue_init();
	Serial.begin(9600);
	delay(100);

	INIT_STATIC_TASK(task1, "producer", NULL, tskIDLE_PRIORITY, 0);
	INIT_STATIC_TASK(task2, "consumer", NULL, tskIDLE_PRIORITY, 1);

	if (!TASK_IS_INITIALIZED(task1) || !TASK_IS_INITIALIZED(task2)) {
		while (true) {
			Serial.println("Error creating tasks");
			delay(500);
		}
	}
}

void loop(void)
{
	Serial.println("LOOP");
	delay(1000);
}


TASK task1(TaskDescriptor_t *self)
{
	while(true) {
		message_t msg = MESSAGE(LOG_STR("Hello There!"), (uint32_t)(self->last_wake));
		message_queue_enqueue(&msg, 100);
		vTaskDelay(50);
	}
}


TASK task2(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();

	while(true) {
		message_t recv;
		// no timeout since this task needs to run at a fixed frequency,
		// if there is no message we just skip this iteration
		message_queue_dequeue(&recv, 0);
		static char buf[256];
		format_message_to_string(&recv, buf, sizeof(buf));
		Serial.println(buf);

		TASK_WAIT_HZ(self, 20);
	}
}
