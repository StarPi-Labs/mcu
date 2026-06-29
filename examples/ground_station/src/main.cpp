#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <FreeRTOS.h>
#include <inttypes.h>

#include "board.h"
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
	lora_setup(BAND_L);


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
		Serial.println("LOOP");
	delay(1000);
}


TASK lora_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();
	uint8_t prev_number = 0;
	bool first_packet = true;

	while (true) {
		if (xSemaphoreTake(spi_semaphore, portMAX_DELAY) == pdTRUE) {
			if (lora_is_reception_done()) {
				LoRaPayload p = lora_get_packet();

				if (!first_packet) {
					uint8_t diff = p.number - prev_number;
					if (diff > 1) {
						Serial.printf("[LoRa] LOST %" PRIu8 " packets (prev #%" PRIu8 " -> #%" PRIu8 ")\n",
							diff - 1, prev_number, p.number);
					}
				}
				first_packet = false;
				prev_number = p.number;

				Serial.printf("[LoRa] Packet #%" PRIu8 "\n", p.number);
				Serial.printf("  timestamp: %" PRIu64 "\n", p.sensor_data.timestamp);
				Serial.printf("  IMU: alt=%.2f vspeed=%.2f att=%.2f dt=%" PRId32 "\n",
					p.sensor_data.imu.altitude,
					p.sensor_data.imu.vspeed,
					p.sensor_data.imu.attitude,
					p.sensor_data.imu.dt);
				Serial.printf("  Baro: alt1=%.2f alt2=%.2f dt=%" PRId32 "\n",
					p.sensor_data.baro.alt1,
					p.sensor_data.baro.alt2,
					p.sensor_data.baro.dt);
				Serial.printf("  GPS: lat=%.6f lon=%.6f\n",
					p.sensor_data.gps.latitude,
					p.sensor_data.gps.longitude);

				lora_start_receive(1000);
			} else {
				// TODO: listen, implement half-duplex communication
			}
			xSemaphoreGive(spi_semaphore);
		}
		TASK_WAIT_HZ(self, LORA_TASK_HZ);
	}
}
