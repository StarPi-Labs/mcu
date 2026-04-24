#include <Arduino.h>
#include <FreeRTOS.h>

#include "board.h"
#include "logger.h"
#include "task.h"
#include "imu.h"
#include "barometer.h"


SPIClass SPI2(FSPI);
TwoWire I2C1(0);


DECLARE_STATIC_SEMAPHORE(spi_semaphore);

DECLARE_STATIC_TASK(imu_task);
DECLARE_STATIC_TASK(barometer_task);
DECLARE_STATIC_TASK(logger_task);


void setup(void)
{

	Serial.begin(115200);
	while (!Serial) {
		delay(100);
	}
	Serial.println("Initialized");

	I2C1.setPins(I2C1_SDA, I2C1_SCL);
	I2C1.begin();
	I2C1.setClock(100000);
	SPI2.begin(SPI2_SCK, SPI2_MISO, SPI2_MOSI, -1);
	// TODO: Set speed

	message_queue_init();

	// for now disable the lora module here
	pinMode(LORA_CS, OUTPUT);
	digitalWrite(LORA_CS, HIGH);

	imu_setup();
	barometer_setup();

	INIT_STATIC_TASK(imu_task, "imu", NULL, tskIDLE_PRIORITY, 0);
	INIT_STATIC_TASK(barometer_task, "barometer", NULL, tskIDLE_PRIORITY, 0);
	INIT_STATIC_TASK(logger_task, "logger", NULL, tskIDLE_PRIORITY, 1);

 	if (!TASK_IS_INITIALIZED(imu_task) || !TASK_IS_INITIALIZED(barometer_task) || !TASK_IS_INITIALIZED(logger_task)) {
		while (true) {
			Serial.println("Error creating tasks");
			delay(500);
		}
	}

	INIT_STATIC_SEMAPHORE(spi_semaphore);
	if (spi_semaphore == NULL) {
		while (true) {
			Serial.println("Error creating semaphore");
			delay(500);
		}
	}
}


void loop(void)
{
	Serial.println("LOOP");
	delay(1000);
}


TASK imu_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();
	FIFO_Sample sample;
	message_t msg;

	while (true) {
		if (xSemaphoreTake(spi_semaphore, portMAX_DELAY) == pdTRUE && imu_get_sample(&sample) == 0) {
			msg = MESSAGE(
				LOG_STR("[IMU]: Accellerometer"),
				(struct ivec3){
					sample.accelerometer[0],
					sample.accelerometer[1],
					sample.accelerometer[2]
				}
			);
			message_queue_enqueue(&msg, 100);

			msg = MESSAGE(
				LOG_STR("[IMU]: Gyroscope"),
				(struct ivec3){
					sample.gyroscope[0],
					sample.gyroscope[1],
					sample.gyroscope[2]
				}
			);
			message_queue_enqueue(&msg, 100);

			// TODO: timestamp
		} else {
			// msg = MESSAGE(LOG_STR("[IMU]: No sample"));
			// message_queue_enqueue(&msg, 100);
		}
		xSemaphoreGive(spi_semaphore);
		TASK_WAIT_HZ(self, IMU_TASK_HZ);
	}
}


TASK barometer_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();
	BaroData sample1, sample2;
	message_t msg;

	while (true) {
		barometer_read(&sample1, &sample2);

		msg = MESSAGE(
			LOG_STR("[BARO1]: Pressure"),
			sample1.pressure
		);
		message_queue_enqueue(&msg, 100);

		msg = MESSAGE(
			LOG_STR("[BARO1]: Altitude"),
			sample1.altitude
		);
		message_queue_enqueue(&msg, 100);

		msg = MESSAGE(
			LOG_STR("[BARO2]: Pressure"),
			sample2.pressure
		);
		message_queue_enqueue(&msg, 100);

		msg = MESSAGE(
			LOG_STR("[BARO2]: Altitude"),
			sample2.altitude
		);
		message_queue_enqueue(&msg, 100);

		TASK_WAIT_HZ(self, BARO_TASK_HZ);
	}
}


TASK logger_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();
	message_t recv;
	static char buf[256];

	while(true) {
		// no timeout since this task needs to run at a fixed frequency,
		// if there is no message we just skip this iteration
		while (message_queue_dequeue(&recv, 0)) {
			format_message_to_string(&recv, buf, sizeof(buf));
			Serial.println(buf);
		}

		TASK_WAIT_HZ(self, LOGGER_TASK_HZ);
	}
}
