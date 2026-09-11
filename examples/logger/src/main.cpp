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
DECLARE_STATIC_TASK(parachute_task);
DECLARE_STATIC_TASK_STACK(gps_task, TASK_STACK_2K);
DECLARE_STATIC_TASK(lora_transmitter_task);
DECLARE_STATIC_TASK(lora_formatter_task);
DECLARE_STATIC_TASK_STACK(uart_task, TASK_STACK_2K);
DECLARE_STATIC_TASK(sd_formatter_task);
DECLARE_STATIC_TASK_STACK(sd_writer_task, TASK_STACK_2K);
DECLARE_STATIC_TASK_STACK(cmd_handler_task, TASK_STACK_2K);

DECLARE_STATIC_QUEUE(parachute_msg_queue, LogMessage, LOG_DEFAULT_QUEUE_SIZE);
DECLARE_STATIC_QUEUE(sd_msg_queue, LogMessage, LOG_DEFAULT_QUEUE_SIZE);
DECLARE_STATIC_QUEUE(uart_msg_queue, LogMessage, LOG_DEFAULT_QUEUE_SIZE);
DECLARE_STATIC_QUEUE(lora_msg_queue, LogMessage, LOG_DEFAULT_QUEUE_SIZE);
DECLARE_STATIC_QUEUE(gs_command_queue, uint64_t, 16);


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
	lora_setup(BAND_L, TX_FORCE, LORA_FC_ID);
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

	INIT_STATIC_QUEUE(parachute_msg_queue);
	INIT_STATIC_QUEUE(sd_msg_queue);
	INIT_STATIC_QUEUE(uart_msg_queue);
	INIT_STATIC_QUEUE(lora_msg_queue);
	INIT_STATIC_QUEUE(gs_command_queue);
	if (parachute_msg_queue == NULL   ||
		sd_msg_queue == NULL          ||
		uart_msg_queue == NULL        ||
		lora_msg_queue == NULL        ||
		gs_command_queue == NULL) {
		while (true) {
			Serial.println("Error creating queues");
			delay(500);
		}
	}

	// Core 0 tasks
	INIT_STATIC_TASK(imu_task, "imu", NULL, tskIDLE_PRIORITY + 10, 0);
	INIT_STATIC_TASK(barometer_task, "barometer", NULL, tskIDLE_PRIORITY + 9, 0);
	INIT_STATIC_TASK(parachute_task, "parachute", NULL, tskIDLE_PRIORITY + 8, 0);
	INIT_STATIC_TASK(gps_task, "gps", NULL, tskIDLE_PRIORITY + 7, 0);
	// Core 1 tasks
	INIT_STATIC_TASK(uart_task, "logger", NULL, tskIDLE_PRIORITY, 1);
	INIT_STATIC_TASK(sd_formatter_task, "sd formatter", NULL, tskIDLE_PRIORITY + 10, 1);
	INIT_STATIC_TASK(sd_writer_task, "sd writer", NULL, tskIDLE_PRIORITY + 9, 1);
	INIT_STATIC_TASK(lora_formatter_task, "lora formatter", NULL, tskIDLE_PRIORITY + 8, 1);
	INIT_STATIC_TASK(lora_transmitter_task, "lora transmitter", NULL, tskIDLE_PRIORITY + 7, 1);
	INIT_STATIC_TASK(cmd_handler_task, "cmd handler", NULL, tskIDLE_PRIORITY + 6, 1);

 	if (
		!TASK_IS_INITIALIZED(imu_task)              ||
		!TASK_IS_INITIALIZED(barometer_task)        ||
		!TASK_IS_INITIALIZED(parachute_task)        ||
		!TASK_IS_INITIALIZED(gps_task)              ||
		!TASK_IS_INITIALIZED(uart_task)             ||
		!TASK_IS_INITIALIZED(lora_transmitter_task) ||
		!TASK_IS_INITIALIZED(lora_formatter_task)   ||
		!TASK_IS_INITIALIZED(sd_formatter_task)     ||
		!TASK_IS_INITIALIZED(cmd_handler_task)      ||
		!TASK_IS_INITIALIZED(sd_writer_task)) {
		while (true) {
			Serial.println("Error creating tasks");
			delay(500);
		}
	}

	// Register consumer tasks to the logger, these tasks will get notified when
	// new data is ready to be read
	logger_register_consumer(parachute_task_descriptor.handle, parachute_msg_queue, 0xffff, T_ALT_SPEED | T_ACCELLERATION);
	logger_register_consumer(lora_formatter_task_descriptor.handle, lora_msg_queue, 0xffff, 0xffff);
	logger_register_consumer(sd_formatter_task_descriptor.handle, sd_msg_queue, 0xffff, 0xffff);
	logger_register_consumer(uart_task_descriptor.handle, uart_msg_queue, 0xffff, 0xffff);

	lora_set_rx_cmd_task_handle(cmd_handler_task_descriptor.handle);
	lora_set_rx_cmd_queue(gs_command_queue);
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

#if TEST_FAKE_DATA == 1
	uint32_t start_time = millis();
#endif

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

#if TEST_FAKE_DATA != 1
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
#else
				#define IDLE_TIME 5000
				static struct DataSample {
					uint32_t time;
					float z_alt;
					float z_speed;
					float z_acc;
				} test_data[] = {
					{ 0         +      0,     0.0,    0.0,   0.0 },
					{ IDLE_TIME +      0,     0.0,    0.0,   0.0 }, // Idle time
					{ IDLE_TIME +    660,     2.0,  35.24,  6.05 }, // Rail exit
					{ IDLE_TIME +   2100,   132.0,  132.0,   7.1 }, // Max g
					{ IDLE_TIME +   4300,   712.7, 251.02, -1.66 }, // Motor burnout
					{ IDLE_TIME +  25700, 3175.22,    0.0, -0.99 }, // Apogee
					{ IDLE_TIME +  28190, 3145.11,  -24.6, -0.83 }, // Drogue deployment
					{ IDLE_TIME +  30000, 2946.39,  -28.2, -0.05 }, // End drogue deployment
					{ IDLE_TIME + 108790,  465.11, -31.71,   0.0 }, // Just before main
					{ IDLE_TIME + 109000,  464.00, -31.71, 13.62 }, // Main deployment
					{ IDLE_TIME + 109150,  462.00,   -5.5,  0.05 }, // End main deployment
					{ IDLE_TIME + 170000,     0.0,    0.0,  0.05 }, // Just before impact
					{ IDLE_TIME + 170100,     0.0,    0.0,   0.0 }, // Touchdown
				};
				#define TEST_DATA_SIZE sizeof(test_data) / sizeof(test_data[0])

				auto get_sample = [&]{
					uint32_t current_time = millis() - start_time;
					if (current_time <= test_data[0].time) {
						return test_data[0];
					}
	
					if (current_time >= test_data[TEST_DATA_SIZE - 1].time) {
						return test_data[TEST_DATA_SIZE - 1];
					}

					for (size_t i = 0; i < TEST_DATA_SIZE - 1; i++) {
						if (current_time >= test_data[i].time && current_time < test_data[i + 1].time) {
							uint32_t t0 = test_data[i].time;
							uint32_t t1 = test_data[i + 1].time;
							
							// Calculate progress fraction between t0 and t1 (0.0 to 1.0)
							float fraction = (float)(current_time - t0) / (float)(t1 - t0);

							auto lerp = [](float start, float end, float frac) {
								return start + frac * (end - start);
							};
	
							DataSample result;
							result.z_alt   = lerp(test_data[i].z_alt,   test_data[i + 1].z_alt,   fraction);
							result.z_speed = lerp(test_data[i].z_speed, test_data[i + 1].z_speed, fraction);
							result.z_acc   = lerp(test_data[i].z_acc,   test_data[i + 1].z_acc,   fraction);
							
							return result;
						}
					}
					return test_data[TEST_DATA_SIZE - 1];
				};

				struct DataSample s = get_sample();

				log(S_IMU, T_ACCELLERATION, 0.0, 0.0, s.z_acc);
				log(S_IMU, T_ALT_SPEED, s.z_alt, s.z_speed);
#endif
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

#if TEST_FAKE_DATA != 1
		log(S_BARO, T_ALT_SPEED, altitude.getState()[0], altitude.getState()[1]);
		log(S_BARO, T_PRESSURE, sample1.pressure, sample2.pressure);
#endif

		TASK_WAIT_HZ(self, BARO_TASK_HZ);
	}
}


TASK parachute_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();
	
	enum RocketState {
		RS_IDLE,      // Idle state, on ramp
		RS_BOOST,     // Motor burning, ascending
		RS_COAST,     // Motor burnt out, still ascending
		RS_DROGUE,    // Drogue deployed, falling
		RS_MAIN,      // Main parachute deployed, falling
		RS_TOUCHDOWN, // On ground
	} state = RS_IDLE;

	#define PARACHUTE_TASK_HZ 10

	#define Z_ACC_BOOST_THRESHOLD_G 2.5
	#define Z_SPEED_BOOST_THRESHOLD_MS 25.0
	#define Z_ALT_BOOST_THRESHOLD_M 100.0

	#define Z_ALT_COAST_THRESHOLD_M 750.0
	#define MOTOR_BURNOUT_MS 4400

	#define Z_SPEED_APOGEE_THRESHOLD_MS 0.5
	#define Z_ALT_APOGEE_THRESHOLD 2950.0
	#define MAX_TIME_TO_APOGEE_MS 28000

	#define MIN_TIME_TO_1500M_MS 8540

	#define Z_ALT_MAIN_DEPLYOMENT_M 450.0
	#define MAX_TIME_TO_MAIN_DEPLOYMENT_MS 110000

	#define Z_ALT_TOUCHDOWN_M 10.0
	#define Z_SPEED_STATIONARY_MS 0.1
	#define MAX_TIME_TO_TOUCHDOWN 200.0

	#define PIN_EJECTION_A  PINT1_LS
	#define PIN_EJECTION_C  PINT2_LS
	#define PIN_MAIN_CUTTER PINT3_LS


	// Interface PINs setup
	pinMode(PIN_EJECTION_A, OUTPUT);
	digitalWrite(PIN_EJECTION_A, 0);
	pinMode(PIN_EJECTION_C, OUTPUT);
	digitalWrite(PIN_EJECTION_C, 0);
	pinMode(PIN_MAIN_CUTTER, OUTPUT);
	digitalWrite(PIN_MAIN_CUTTER, 0);


	// Sensor data each loop
	float z_speed = 0;
	float z_alt = 0;
	float z_acc = 0;

	// Milliseconds since detected motor ignition
	int64_t ms_since_ignition = 0;
	// Time of detected motor ignition
	int64_t ms_ignition = 0;

	while (true) {
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000/PARACHUTE_TASK_HZ));

		// Get the sensor data, if there is no new sensor data the old sample is
		// used
		LogMessage msg;
		while (xQueueReceive(parachute_msg_queue, &msg, 0) == pdTRUE) {
			if (msg.type == T_ALT_SPEED && msg.payload_type == P_FVEC2) {
				z_alt = msg.payload.fv2.x;
				z_speed = msg.payload.fv2.y;
			} else if (msg.type == T_ACCELLERATION && msg.payload_type == P_FVEC3) {
				z_acc = msg.payload.fv3.z;
			}
		}

		// FIXME: will this always work?
		ms_since_ignition = millis() - ms_ignition;

		switch (state) {
		case RS_IDLE:
			// Detect motor ignition
			if (z_acc >= Z_ACC_BOOST_THRESHOLD_G || z_speed >= Z_SPEED_BOOST_THRESHOLD_MS || z_alt >= Z_ALT_BOOST_THRESHOLD_M) {
				ms_ignition = millis();
				state = RS_BOOST;
			}
			log(S_PARA, T_SYSLOG, "State: RS_IDLE");
			break;

		case RS_BOOST:
			// Detect motor burnout
			if (z_alt >= Z_ALT_COAST_THRESHOLD_M || ms_since_ignition >= MOTOR_BURNOUT_MS) {
				state = RS_COAST;
			}
			log(S_PARA, T_SYSLOG, "State: RS_BOOST");
			break;

		case RS_COAST:
			if (ms_since_ignition >= MIN_TIME_TO_1500M_MS) {
				// TODO: control aibrakes
			}
			// Detect apogee
			if (z_speed <= Z_SPEED_APOGEE_THRESHOLD_MS || z_alt <= Z_ALT_APOGEE_THRESHOLD || ms_since_ignition >= MAX_TIME_TO_APOGEE_MS) {
				// Activate recovery A and C
				analogWrite(PIN_EJECTION_A, 256/2);
				digitalWrite(PIN_EJECTION_C, 1);
				vTaskDelay(pdMS_TO_TICKS(2000));
				digitalWrite(PIN_EJECTION_A, 0);
				digitalWrite(PIN_EJECTION_C, 0);

				state = RS_DROGUE;
			}
			log(S_PARA, T_SYSLOG, "State: RS_COAST");
			break;

		case RS_DROGUE:
			// TODO: retract airbrakes

			if (z_alt <= Z_ALT_MAIN_DEPLYOMENT_M || ms_since_ignition >= MAX_TIME_TO_MAIN_DEPLOYMENT_MS) {
				// Cut main parachute
				analogWrite(PIN_MAIN_CUTTER, 256/2);
				vTaskDelay(pdMS_TO_TICKS(2000));
				digitalWrite(PIN_MAIN_CUTTER, 0);
				
				state = RS_MAIN;
			}

			log(S_PARA, T_SYSLOG, "State: RS_DROGUE");
			break;

		case RS_MAIN:
			if (z_alt <= Z_ALT_TOUCHDOWN_M || z_speed <= Z_SPEED_STATIONARY_MS || ms_since_ignition >= MAX_TIME_TO_TOUCHDOWN) {
				state = RS_TOUCHDOWN;
			}
			log(S_PARA, T_SYSLOG, "State: RS_MAIN");
			break;

		case RS_TOUCHDOWN:
			log(S_PARA, T_SYSLOG, "State: RS_TOUCHDOWN");
			break;

		default:
			log(S_PARA, T_SYSLOG, "[ERR]: Unknown rocket state");
			break;
		}
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

		LoRaDataPacket *dp = lora_get_tx_packet();
		while (xQueueReceive(lora_msg_queue, &msg, 0) == pdTRUE) {
			switch (msg.type) {
			case T_ALT_SPEED:
				if (msg.payload_type == P_FVEC2) {
					dp->imu.altitude = float16(msg.payload.fv2.x).getBinary();
					dp->imu.vspeed = float16(msg.payload.fv2.y).getBinary();
					dp->imu.dt = msg.timestamp/1000 - u48le_to_u64(dp->header.tx_time);
				}
				break;
			case T_ORIENTATION: {
				if (msg.payload_type == P_FVEC3) {
					// FIXME: don't repeat this computation here
					float r = msg.payload.fv3.x * 0.0174533; // roll in radians
					float p = msg.payload.fv3.y * 0.0174533; // pitch in radians
					float a = acos(cos(p)*cos(r)) * 57.2958; // total pitch from vertical in degrees

					dp->imu.attitude = float16(a).getBinary();
					dp->imu.dt = msg.timestamp/1000 - u48le_to_u64(dp->header.tx_time);
				}
				break;
			}
			case T_PRESSURE:
				if (msg.payload_type == P_FVEC2) {
					dp->baro.p1 = float16(msg.payload.fv2.x).getBinary();
					dp->baro.p2 = float16(msg.payload.fv2.y).getBinary();
					dp->baro.dt = msg.timestamp/1000 - u48le_to_u64(dp->header.tx_time);
				}
				break;
			case T_GPS:
				if (msg.payload_type == P_FVEC2) {
					dp->gps.latitude = msg.payload.fv2.x;
					dp->gps.longitude = msg.payload.fv2.y;
					dp->gps.dt = msg.timestamp/1000 - u48le_to_u64(dp->header.tx_time);
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

	while (true) {
		// Run the lora radio state machine
		LoRaFCState state = lora_fc_state_machine();
		String str;
		switch(state) {
		case STATE_DISCONNECTED:
			str = "STATE_DISCONNECTED";
			break;
		case STATE_CONNECTING:
			str = "STATE_CONNECTING";
			break;
		case STATE_TRANSMIT:
			str = "STATE_TRANSMIT";
			break;
		case STATE_RECEIVE:
			str = "STATE_RECEIVE";
			break;
		default:
			str = "UNKNOWN";
			break;
		}
		Serial.println(str);
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


TASK cmd_handler_task(TaskDescriptor_t *self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
		ulTaskNotifyTake(pdTRUE, 0);
		uint64_t cmd;
		xQueueReceive(gs_command_queue, &cmd, 0);
		Serial.printf("GROUND STATION COMMAND: %llu\n", cmd);
	}
}
