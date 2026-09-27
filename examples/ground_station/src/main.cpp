#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <FreeRTOS.h>
#include <inttypes.h>

#include "board.h"
#include "logger.h"
#include "task.h"
#include "lora.h"


SPIClass SPI2(FSPI);
TwoWire I2C1(0);

DECLARE_STATIC_SEMAPHORE(spi_semaphore);
// TODO: add semaphore for i2c once we connect the pitot

DECLARE_STATIC_TASK(lora_task);

void setup(void)
{

	Serial.begin(115200);
//	while (!Serial) {
//		delay(100);
//	}
	Serial.println("Initialized");

	SPI2.begin(SPI2_SCK, SPI2_MISO, SPI2_MOSI, -1);
	lora_setup(BAND_L, TX_FORCE, LORA_GS_ID);

	INIT_STATIC_SEMAPHORE(spi_semaphore);
	if (spi_semaphore == NULL) {
		while (true) {
			Serial.println("Error creating semaphore");
			delay(500);
		}
	}

	// Core 0 tasks
	// Core 1 tasks
	INIT_STATIC_TASK(lora_task, "lora", NULL, tskIDLE_PRIORITY + 10, 1);

 	if (!TASK_IS_INITIALIZED(lora_task)) {
		while (true) {
			Serial.println("Error creating tasks");
			delay(500);
		}
	}
}


void loop(void)
{
	if (Serial)
		Serial.println("GROUND STATION LOOP");
	delay(1000);
}


TASK lora_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
//		if (xSemaphoreTake(spi_semaphore, portMAX_DELAY) == pdTRUE) {
		LOG("state = %d", lora_gs_state_machine());
//			xSemaphoreGive(spi_semaphore);
//		}
	}
}
