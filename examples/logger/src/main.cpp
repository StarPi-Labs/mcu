#include <Arduino.h>
#include <FreeRTOS.h>
#include <Adafruit_AHRS.h>

#include "board.h"
#include "logger.h"
#include "task.h"
#include "imu.h"
#include "barometer.h"
#include "lora.h"
#include "sdcard.h"
#include "KalmanFilter.hpp"
#include "gps.h"


SPIClass SPI2(FSPI);
TwoWire I2C1(0);

// State filter for orientation estimation, used in the IMU task
Adafruit_Mahony orientation;

// Altitude and veritcal velocity state filter, used to sense when to deploy
// the parachute
KalmanFilter altitude;

extern float g_cal;

DECLARE_STATIC_SEMAPHORE(spi_semaphore);
// TODO: add semaphore for i2c once we connect the pitot

DECLARE_STATIC_TASK(imu_task);
DECLARE_STATIC_TASK(barometer_task);
DECLARE_STATIC_TASK(gps_task);
DECLARE_STATIC_TASK(lora_transmitter_task);
DECLARE_STATIC_TASK(lora_formatter_task);
DECLARE_STATIC_TASK(uart_task);
DECLARE_STATIC_TASK(sd_formatter_task);
DECLARE_STATIC_TASK(sd_writer_task);

DECLARE_STATIC_QUEUE(sd_msg_queue, LogMessage, 128);
DECLARE_STATIC_QUEUE(uart_msg_queue, LogMessage, 128);
DECLARE_STATIC_QUEUE(lora_msg_queue, LogMessage, 128);


void setup(void)
{

	Serial.begin(115200);
//	while (!Serial) {
//		delay(100);
//	}
	Serial.println("Initialized");

	I2C1.setPins(I2C1_SDA, I2C1_SCL);
	I2C1.begin();
	I2C1.setClock(100000);
	SPI2.begin(SPI2_SCK, SPI2_MISO, SPI2_MOSI, -1);
	// TODO: Set speed

	logger_init();

	imu_setup();
	altitude.setG(g_cal);

	barometer_setup();
	lora_setup(BAND_L, TX_FORCE);
	gps_setup();

	if (!sdcard_init()) {
		while (true) {
			Serial.println("SD init failed");
			delay(500);
		}
	}

	// creates folder /session_<session_num>
	if (!sdcard_start_session()) {
		while (true) {
			Serial.println("Error creating the SD session folder");
			delay(500);
		}
	}

	// FIXME: to make sure that data is always saved the log file should be flush, opened and closed sometimes,
	// maybe add a log rotation
	sdcard_open_log();

	INIT_STATIC_SEMAPHORE(spi_semaphore);
	if (spi_semaphore == NULL) {
		while (true) {
			Serial.println("Error creating semaphore");
			delay(500);
		}
	}

	INIT_STATIC_QUEUE(sd_msg_queue);
	INIT_STATIC_QUEUE(uart_msg_queue);
	INIT_STATIC_QUEUE(lora_msg_queue);
	if (sd_msg_queue == NULL   ||
	    uart_msg_queue == NULL ||
	    lora_msg_queue == NULL) {
		while (true) {
			Serial.println("Error creating queues");
			delay(500);
		}
	}

	// Core 0 tasks
	INIT_STATIC_TASK(imu_task, "imu", NULL, tskIDLE_PRIORITY + 10, 0);
	INIT_STATIC_TASK(barometer_task, "barometer", NULL, tskIDLE_PRIORITY + 9, 0);
	INIT_STATIC_TASK(gps_task, "gps", NULL, tskIDLE_PRIORITY + 8, 0);
	// Core 1 tasks
	INIT_STATIC_TASK(uart_task, "logger", NULL, tskIDLE_PRIORITY, 1);
	INIT_STATIC_TASK(sd_formatter_task, "sd formatter", NULL, tskIDLE_PRIORITY + 10, 1);
	INIT_STATIC_TASK(sd_writer_task, "sd writer", NULL, tskIDLE_PRIORITY + 9, 1);
	INIT_STATIC_TASK(lora_formatter_task, "lora formatter", NULL, tskIDLE_PRIORITY + 8, 1);
	INIT_STATIC_TASK(lora_transmitter_task, "lora transmitter", NULL, tskIDLE_PRIORITY + 7, 1);

 	if (
	    !TASK_IS_INITIALIZED(imu_task)              ||
	    !TASK_IS_INITIALIZED(barometer_task)        ||
	    !TASK_IS_INITIALIZED(gps_task)              ||
	    !TASK_IS_INITIALIZED(uart_task)             ||
	    !TASK_IS_INITIALIZED(lora_transmitter_task) ||
	    !TASK_IS_INITIALIZED(lora_formatter_task)   ||
	    !TASK_IS_INITIALIZED(sd_formatter_task)     ||
	    !TASK_IS_INITIALIZED(sd_writer_task)) {
		while (true) {
			Serial.println("Error creating tasks");
			delay(500);
		}
	}

	// Register consumer tasks to the logger, these tasks will get notified when
	// new data is ready to be read
	logger_register_consumer(lora_formatter_task_descriptor.handle, lora_msg_queue, 0xffff, 0xffff);
	logger_register_consumer(sd_formatter_task_descriptor.handle, sd_msg_queue, 0xffff, 0xffff);
	logger_register_consumer(uart_task_descriptor.handle, uart_msg_queue, 0xffff, 0xffff);
}


void loop(void)
{
//	if (Serial)
//		Serial.println("LOOP");
	delay(1000);
}


TASK imu_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();
	FIFO_Sample sample;
	orientation.begin(IMU_TASK_HZ);

	while (true) {
		if (xSemaphoreTake(spi_semaphore, portMAX_DELAY) == pdTRUE) {
			if(imu_get_sample(&sample) == 0) {
				// Update the relative orientation of the board using
				// the Madgwick filter, readings are in mg and mdps, so
				// conversion is needed
				orientation.updateIMU(
					(float)sample.gyroscope[0]/1000.0f,
					(float)sample.gyroscope[1]/1000.0f,
					(float)sample.gyroscope[2]/1000.0f,
					(float)sample.accelerometer[0]/1000.0f,
					(float)sample.accelerometer[1]/1000.0f,
					(float)sample.accelerometer[2]/1000.0f
				);

				// Update the altitude and vertical velocity estimation
				// with the inertial data
				float attitude_rad = acos(cos(orientation.getPitchRadians())*cos(orientation.getRollRadians()));
				altitude.predict(
					(float)sample.accelerometer[2]/1000.0f,
					attitude_rad,
					false // TODO: airbrake trigger
				);

				log(S_IMU, T_ORIENTATION,
					orientation.getRoll(),
					orientation.getPitch(),
					orientation.getYaw()
				);
				log(S_IMU, T_ACCELLERATION,
					sample.accelerometer[0],
					sample.accelerometer[1],
					sample.accelerometer[2]
				);
				log(S_IMU, T_GYRO,
					sample.gyroscope[0],
					sample.gyroscope[1],
					sample.gyroscope[2]
				);
				log(S_IMU, T_ALT_SPEED, altitude.getState()[0], altitude.getState()[1]);
			}
			xSemaphoreGive(spi_semaphore);
		}
		TASK_WAIT_HZ(self, IMU_TASK_HZ);
	}
}


TASK barometer_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();
	BaroData sample1, sample2;

	while (true) {
		barometer_read(&sample1, &sample2);

		// FIXME: is the median really the best way to fuse the two barometer readings?
		float alt = (sample1.altitude + sample2.altitude) / 2.0f;

		// Update the altitude and vertical velocity estimation with the barometer data
		altitude.update(alt);

		log(S_BARO, T_ALT_SPEED, altitude.getState()[0], altitude.getState()[1]);
		log(S_BARO, T_PRESSURE, sample1.pressure, sample2.pressure);

		TASK_WAIT_HZ(self, BARO_TASK_HZ);
	}
}


TASK gps_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();
	static GPSData data;

	while(true) {
		gps_update(&data);

		if (data.num_sat < GPS_MIN_SATELLITES) {
			// TODO: print satellites
			log(S_GPS, T_SYSLOG, "Not enough satellites");
		} else {
			//LOG("[GPS]: (%d) pos: (%f, %f), alt: %fm, speed: %fkmh, time:%llu",
			//	data.num_sat, data.lat, data.lon, data.alt, data.kmh, data.unix_time);
			log(S_GPS, T_GPS, data.lat, data.lon);
		}

		TASK_WAIT_HZ(self, GPS_TASK_HZ);
	}
}


TASK lora_formatter_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000/LORA_FMT_TASK_HZ));

		LogMessage msg;

		LoRaPayload *pl = lora_get_tx_packet();
		while (xQueueReceive(lora_msg_queue, &msg, 0) == pdTRUE) {
			switch (msg.type) {
			case T_ALT_SPEED:
				if (msg.payload_type == P_FVEC2) {
					pl->data.imu.altitude = float16(msg.payload.fv2.x);
					pl->data.imu.vspeed = float16(msg.payload.fv2.y);
					pl->data.imu.dt = lora_compute_dt(pl, msg.timestamp);
				}
				break;
			case T_ORIENTATION: {
				if (msg.payload_type == P_FVEC3) {
					// FIXME: don't repeat this computation here
					float r = msg.payload.fv3.x * 0.0174533; // roll in radians
					float p = msg.payload.fv3.y * 0.0174533; // pitch in radians
					float a = acos(cos(p)*cos(r)) * 57.2958; // total pitch from vertical in degrees

					pl->data.imu.attitude = float16(a);
					pl->data.imu.dt = lora_compute_dt(pl, msg.timestamp);
				}
				break;
			}
			case T_PRESSURE:
				if (msg.payload_type == P_FVEC2) {
					pl->data.baro.p1 = float16(msg.payload.fv2.x);
					pl->data.baro.p2 = float16(msg.payload.fv2.y);
					pl->data.baro.dt = lora_compute_dt(pl, msg.timestamp);
				}
				break;
			case T_GPS:
				if (msg.payload_type == P_FVEC2) {
					pl->data.gps.latitude = msg.payload.fv2.x;
					pl->data.gps.longitude = msg.payload.fv2.y;
					pl->data.gps.dt = lora_compute_dt(pl, msg.timestamp);
				}
				break;
			// TODO: append syslog
			default:
				break;
			}
		}
		lora_release_tx_packet();
	}
}


TASK lora_transmitter_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();
	uint8_t packet_num = 0;

	while (true) {
		if (lora_is_transmission_done()) {
			if (xSemaphoreTake(spi_semaphore, portMAX_DELAY) == pdTRUE) {
				//LOG("[LoRa]: Sending packet %d", packet_num);
				lora_start_transmission();
				lora_prepare_next_packet(packet_num++);
				xSemaphoreGive(spi_semaphore);
			}
		} else {
			// TODO: listen, implement half-duplex communication
		}
		TASK_WAIT_HZ(self, LORA_TX_TASK_HZ);
	}
}


// UART consumer
TASK uart_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();

	while(true) {
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000/UART_TASK_HZ));
		LogMessage msg;
		const char *str;
		size_t len = 0;
		while (xQueueReceive(uart_msg_queue, &msg, 0) == pdTRUE) {
			len = logger_message_to_str(&str, &msg);
			Serial.write(str, len);
		}
	}
}


// SD consumer
TASK sd_formatter_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();

	while(true) {
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000/SD_FMT_TASK_HZ));
		LogMessage msg;

		while (xQueueReceive(sd_msg_queue, &msg, 0) == pdTRUE) {
			const char *str = NULL;
			size_t len = logger_message_to_str(&str, &msg);
			sdcard_write(str, len);
		}
	}
}


TASK sd_writer_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
		sdcard_flush();
		TASK_WAIT_HZ(self, SD_WRITER_TASK_HZ);
	}
}
