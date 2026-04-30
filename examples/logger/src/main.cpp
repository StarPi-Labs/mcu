#include <Arduino.h>
#include <FreeRTOS.h>
#include <MadgwickAHRS.h>

#include "board.h"
#include "logger.h"
#include "task.h"
#include "imu.h"
#include "barometer.h"
#include "lora.h"
#include "KalmanFilter2.hpp"


SPIClass SPI2(FSPI);
TwoWire I2C1(0);

// State filter for orientation estimation, used in the IMU task
Madgwick orientation;

DECLARE_STATIC_SEMAPHORE(spi_semaphore);
// TODO: add semaphore for i2c once we connect the pitot

DECLARE_STATIC_TASK(imu_task);
DECLARE_STATIC_TASK(barometer_task);
DECLARE_STATIC_TASK(lora_task);
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

	imu_setup();

	barometer_setup();
	lora_setup();


	INIT_STATIC_TASK(imu_task, "imu", NULL, tskIDLE_PRIORITY, 0);
	INIT_STATIC_TASK(barometer_task, "barometer", NULL, tskIDLE_PRIORITY, 0);
	INIT_STATIC_TASK(logger_task, "logger", NULL, tskIDLE_PRIORITY, 1);
	INIT_STATIC_TASK(lora_task, "lora", NULL, tskIDLE_PRIORITY, 1);

 	if (
	    !TASK_IS_INITIALIZED(imu_task) ||
	    !TASK_IS_INITIALIZED(barometer_task) ||
	    !TASK_IS_INITIALIZED(logger_task) ||
	    !TASK_IS_INITIALIZED(lora_task)) {
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
	KalmanFilter& altitude = KalmanFilter::getInstance();
	message_t msg;
	orientation.begin(IMU_TASK_HZ);

	while (true) {
		if (xSemaphoreTake(spi_semaphore, portMAX_DELAY) == pdTRUE && imu_get_sample(&sample) == 0) {
			// Update the relative orientation of the board using
			// the Madgwick filter, readings are in mg and mdps, so
			// conversion is needed
			altitude.predict(
				(float)sample.gyroscope[0] * 1e-3f * 0.01745329f,
				(float)sample.gyroscope[1] * 1e-3f * 0.01745329f,
				(float)sample.gyroscope[2] * 1e-3f * 0.01745329f,
				(float)sample.accelerometer[0] * 9.81e-3f,
				(float)sample.accelerometer[1] * 9.81e-3f,
				(float)sample.accelerometer[2] * 9.81e-3f
			);

			/* msg = MESSAGE(
				LOG_STR("[IMU]: Orientation"),
				(struct vec3){
					orientation.getRoll(),
					orientation.getPitch(),
					orientation.getYaw(),
				}
			); 
			message_queue_enqueue(&msg, 100); */

			xSemaphoreGive(spi_semaphore);
		}
		TASK_WAIT_HZ(self, IMU_TASK_HZ);
	}
}


TASK barometer_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();
	BaroData sample1, sample2;
	KalmanFilter& altitude = KalmanFilter::getInstance();
	message_t msg;

	while (true) {
		barometer_read(&sample1, &sample2);

		//float alt = (sample1.altitude + sample2.altitude) / 2.0f;

		// Update the altitude and vertical velocity estimation with the barometer data
		altitude.update(sample1.pressure);

		message_queue_enqueue(&msg, 100);

		TASK_WAIT_HZ(self, BARO_TASK_HZ);
	}
}


TASK lora_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
/*
		if (xSemaphoreTake(spi_semaphore, portMAX_DELAY) == pdTRUE) {
			if (lora_is_transmission_done()) {
				LOG("[LORA]: Transmission done");
				lora_start_transmission(LoRaPayload{0}.bytes, sizeof(LoRaPayload));
			} else {
				LOG("[LORA]: Transmission in progress");
			}
			xSemaphoreGive(spi_semaphore);
		}
*/
		TASK_WAIT_HZ(self, LORA_TASK_HZ);
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
