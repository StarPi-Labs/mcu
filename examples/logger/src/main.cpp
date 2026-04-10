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

	//if (thandle_task1 == NULL || thandle_task2 == NULL) {
	if (false) {
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
		message_t msg = {
			.timestamp = 1234,
			.type = MSG_NONE,
			.description = LOG_STR("Hello There!"),
		};
		message_queue_enqueue(&msg, 100);
		vTaskDelay(50);
	}
}


TASK task2(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();

	while(true) {
		message_t recv;
		message_queue_dequeue(&recv, 100);
		Serial.println(recv.description);

		TASK_WAIT_HZ(self, 20);
	}
}
