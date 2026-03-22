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

	INIT_STATIC_TASK(task1, "producer", NULL, tskIDLE_PRIORITY);
	INIT_STATIC_TASK(task2, "consumer", NULL, tskIDLE_PRIORITY);

	if (thandle_task1 == NULL || thandle_task2 == NULL) {
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


void task1(void* data)
{
	// TASKS SHOULD NEVER RETURN
	// TODO: Wrap tasks such that they can return
	while(true) {
		(void)data;
		message_t msg = {
			.timestamp = 1234,
			.type = MSG_NONE,
			.description = LOG_STR("Hello There!"),
		};
		message_queue_enqueue(&msg, 100);
		vTaskDelay(50);
	}
}


void task2(void *data)
{
	while(true) {
		(void)data;
		message_t recv;
		message_queue_dequeue(&recv, 100);
		Serial.println(recv.description);
		vTaskDelay(20);
	}
}
