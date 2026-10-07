#include <Adafruit_AHRS.h>
#include <Arduino.h>
#include <FreeRTOS.h>

#include "Bluetooth.hpp"
#include "KalmanFilter.hpp"
#include "barometer.h"
#include "board.h"
#include "gps.h"
#include "imu.h"
#include "logger.h"
#include "lora.h"
#include "sdcard.h"
#include "task.h"

SPIClass SPI2(FSPI);
TwoWire I2C1(0);

// State filter for orientation estimation, used in the IMU task
Adafruit_Mahony orientation;

// Altitude and veritcal velocity state filter, used to sense when to deploy
// the parachute
KalmanFilter altitude;

extern float g_cal;

// TX lora packet
LoRaDataPacket lora_tx_packet;
DECLARE_STATIC_SEMAPHORE(lora_tx_packet_semaphore);

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
DECLARE_STATIC_TASK(ble_formatter_task);
DECLARE_STATIC_TASK_STACK(cmd_handler_task, TASK_STACK_2K);

DECLARE_STATIC_QUEUE(parachute_msg_queue, LogMessage, LOG_DEFAULT_QUEUE_SIZE);
DECLARE_STATIC_QUEUE(sd_msg_queue, LogMessage, LOG_DEFAULT_QUEUE_SIZE);
DECLARE_STATIC_QUEUE(uart_msg_queue, LogMessage, LOG_DEFAULT_QUEUE_SIZE);
DECLARE_STATIC_QUEUE(lora_msg_queue, LogMessage, LOG_DEFAULT_QUEUE_SIZE);
DECLARE_STATIC_QUEUE(ble_msg_queue, LogMessage, LOG_DEFAULT_QUEUE_SIZE);

// A ground station command, from either radio. Both the BLE write callback and the LoRa RX callback push into
// gs_command_queue, cmd_handler_task is the only reader.
struct GsCommand {
	SourceSubsystem src; // radio it arrived on: S_BLE or S_LORA
	uint8_t command;     // raw id as received (see LoRaCommand), validated by cmd_handler_task
	uint64_t data;       // LoRaCommandPacket::data, always 0 over BLE
};
DECLARE_STATIC_QUEUE(gs_command_queue, GsCommand, 16);

// Queue a ground station command for cmd_handler_task. Never blocks: it is called from the NimBLE host task and
// from the LoRa state machine. Task context only, not ISR safe.
static void gs_command_enqueue(SourceSubsystem src, uint8_t command, uint64_t data)
{
	GsCommand cmd = {.src = src, .command = command, .data = data};
	if (xQueueSend(gs_command_queue, &cmd, 0) != pdTRUE)
		log(src, T_SYSLOG, "Command queue full, command dropped");
}

bool lora_tx_cb(uint8_t* packet)
{
	xSemaphoreTake(lora_tx_packet_semaphore, portMAX_DELAY);
	*((LoRaDataPacket*)packet) = lora_tx_packet;
	u64_to_u48le(now_ms(), lora_tx_packet.header.tx_time);
	xSemaphoreGive(lora_tx_packet_semaphore);
	return true;
}

void lora_rx_cb(uint8_t* packet)
{
	// Called by lora_fc_state_machine(), i.e. from lora_transmitter_task: task context, not an ISR
	LoRaCommandPacket cmd;
	memcpy(&cmd, packet, sizeof(cmd));
	gs_command_enqueue(S_LORA, (uint8_t)cmd.command, cmd.data);
}

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

	INIT_STATIC_SEMAPHORE(spi_semaphore);
	INIT_STATIC_SEMAPHORE(lora_tx_packet_semaphore);
	if (spi_semaphore == NULL || lora_tx_packet_semaphore == NULL) {
		while (true) {
			Serial.println("Error creating semaphore");
			delay(500);
		}
	}

	logger_init();

	// Created before the radios are started: a BLE write can arrive as soon as ble_init() returns
	INIT_STATIC_QUEUE(gs_command_queue);
	if (gs_command_queue == NULL) {
		while (true) {
			Serial.println("Error creating queues");
			delay(500);
		}
	}

	ble_init({.on_command =
	              [](uint8_t command, void* context) {
		              (void)context;
		              gs_command_enqueue(S_BLE, command, 0);
	              },
	          .context = nullptr});

	imu_setup();
	altitude.setG(g_cal);

	barometer_setup();

	lora_setup(BAND_L, TX_FORCE, LORA_FC_ID);
	lora_set_tx_packet_cb(lora_tx_cb);
	lora_set_rx_packet_cb(lora_rx_cb);

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

	INIT_STATIC_QUEUE(parachute_msg_queue);
	INIT_STATIC_QUEUE(sd_msg_queue);
	INIT_STATIC_QUEUE(uart_msg_queue);
	INIT_STATIC_QUEUE(lora_msg_queue);
	INIT_STATIC_QUEUE(ble_msg_queue);
	if (parachute_msg_queue == NULL || sd_msg_queue == NULL || uart_msg_queue == NULL || lora_msg_queue == NULL ||
	    ble_msg_queue == NULL) {
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
	INIT_STATIC_TASK(sd_formatter_task, "sd formatter", NULL, tskIDLE_PRIORITY + 10, 1);
	INIT_STATIC_TASK(sd_writer_task, "sd writer", NULL, tskIDLE_PRIORITY + 9, 1);
	INIT_STATIC_TASK(lora_formatter_task, "lora formatter", NULL, tskIDLE_PRIORITY + 8, 1);
	INIT_STATIC_TASK(lora_transmitter_task, "lora transmitter", NULL, tskIDLE_PRIORITY + 7, 1);
	INIT_STATIC_TASK(cmd_handler_task, "cmd handler", NULL, tskIDLE_PRIORITY + 6, 1);
	INIT_STATIC_TASK(ble_formatter_task, "ble formatter", NULL, tskIDLE_PRIORITY + 5, 1);
	INIT_STATIC_TASK(uart_task, "logger", NULL, tskIDLE_PRIORITY, 1);

	if (!TASK_IS_INITIALIZED(imu_task) || !TASK_IS_INITIALIZED(barometer_task) ||
	    !TASK_IS_INITIALIZED(parachute_task) || !TASK_IS_INITIALIZED(gps_task) || !TASK_IS_INITIALIZED(uart_task) ||
	    !TASK_IS_INITIALIZED(lora_transmitter_task) || !TASK_IS_INITIALIZED(lora_formatter_task) ||
	    !TASK_IS_INITIALIZED(sd_formatter_task) || !TASK_IS_INITIALIZED(ble_formatter_task) ||
	    !TASK_IS_INITIALIZED(cmd_handler_task) || !TASK_IS_INITIALIZED(sd_writer_task)) {
		while (true) {
			Serial.println("Error creating tasks");
			delay(500);
		}
	}

	// Register consumer tasks to the logger, these tasks will get notified when
	// new data is ready to be read
	logger_register_consumer(parachute_task_descriptor.handle, parachute_msg_queue, 0xffff, T_FILTER_STATE);
	logger_register_consumer(lora_formatter_task_descriptor.handle, lora_msg_queue, 0xffff, 0xffff);
	logger_register_consumer(sd_formatter_task_descriptor.handle, sd_msg_queue, 0xffff, 0xffff);
	logger_register_consumer(uart_task_descriptor.handle, uart_msg_queue, 0xffff, 0xffff);
	logger_register_consumer(ble_formatter_task_descriptor.handle, ble_msg_queue, 0xffff, 0xffff);

	// set a debug led after initialization
	pinMode(PINT5_LS, OUTPUT);
	digitalWrite(PINT5_LS, 1);
}

void loop(void)
{
	//	if (Serial)
	//		Serial.println("LOOP");
	delay(1000);
}

TASK imu_task(TaskDescriptor_t* self)
{
	self->last_wake = xTaskGetTickCount();
	FIFO_Sample sample;
	orientation.begin(IMU_TASK_HZ);

#if TEST_FAKE_DATA == 1
	uint32_t start_time = millis();
#endif

	while (true) {
		if (xSemaphoreTake(spi_semaphore, portMAX_DELAY) == pdTRUE) {
			if (imu_get_sample(&sample) == 0) {
				float ax = (float)sample.accelerometer[0] / 1000.0f;
				float ay = (float)sample.accelerometer[1] / 1000.0f;
				float az = (float)sample.accelerometer[2] / 1000.0f;
				float gx = (float)sample.gyroscope[0] / 1000.0f;
				float gy = (float)sample.gyroscope[1] / 1000.0f;
				float gz = (float)sample.gyroscope[2] / 1000.0f;

				// Mahony filter quaternions
				float qw, qx, qy, qz;

				// Update the relative orientation of the board using
				// the Mahony filter, readings are in mg and mdps, so
				// conversion is needed
				orientation.updateIMU(gx, gy, gz, ax, ay, az);
				orientation.getQuaternion(&qw, &qx, &qy, &qz);

				// Update the altitude and vertical velocity estimation
				// with the inertial data
				float cos_tilt = constrain(1 - 2*(qx*qx + qy*qy), -1.0f, 1.0f);
				float attitude_rad = acosf(cos_tilt); // same as acos(cos p * cos r) but NaN-safe
				float vertical_accel = 2*(qx*qz - qw*qy) * ax + 2*(qy*qz + qw*qx) * ay + (1 - 2*(qx*qx + qy*qy)) * az;
				// TODO: airbrake trigger
				altitude.predict(vertical_accel, attitude_rad, false);

#if TEST_FAKE_DATA != 1
				log(S_IMU, T_ORIENTATION, orientation.getRoll(), orientation.getPitch(), orientation.getYaw());
				log(S_IMU, T_ACCELLERATION, sample.accelerometer[0], sample.accelerometer[1], sample.accelerometer[2]);
				log(S_IMU, T_GYRO, sample.gyroscope[0], sample.gyroscope[1], sample.gyroscope[2]);
				log(S_IMU, T_FILTER_STATE, altitude.getState()[0], altitude.getState()[1], altitude.getState()[2]);
#else
#define IDLE_TIME 5000
				static struct DataSample {
					uint32_t time;
					float z_alt;
					float z_speed;
					float z_acc;
				} test_data[] = {
				    {0 + 0, 0.0, 0.0, 0.0},
				    {IDLE_TIME + 0, 0.0, 0.0, 0.0},              // Idle time
				    {IDLE_TIME + 660, 2.0, 35.24, 6.05},         // Rail exit
				    {IDLE_TIME + 2100, 132.0, 132.0, 7.1},       // Max g
				    {IDLE_TIME + 4300, 712.7, 251.02, -1.66},    // Motor burnout
				    {IDLE_TIME + 25700, 3175.22, 0.0, -0.99},    // Apogee
				    {IDLE_TIME + 28190, 3145.11, -24.6, -0.83},  // Drogue deployment
				    {IDLE_TIME + 30000, 2946.39, -28.2, -0.05},  // End drogue deployment
				    {IDLE_TIME + 108790, 465.11, -31.71, 0.0},   // Just before main
				    {IDLE_TIME + 109000, 464.00, -31.71, 13.62}, // Main deployment
				    {IDLE_TIME + 109150, 462.00, -5.5, 0.05},    // End main deployment
				    {IDLE_TIME + 170000, 0.0, 0.0, 0.05},        // Just before impact
				    {IDLE_TIME + 170100, 0.0, 0.0, 0.0},         // Touchdown
				};
#define TEST_DATA_SIZE sizeof(test_data) / sizeof(test_data[0])

				auto get_sample = [&] {
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

							auto lerp = [](float start, float end, float frac) { return start + frac * (end - start); };

							DataSample result;
							result.z_alt = lerp(test_data[i].z_alt, test_data[i + 1].z_alt, fraction);
							result.z_speed = lerp(test_data[i].z_speed, test_data[i + 1].z_speed, fraction);
							result.z_acc = lerp(test_data[i].z_acc, test_data[i + 1].z_acc, fraction);

							return result;
						}
					}
					return test_data[TEST_DATA_SIZE - 1];
				};

				struct DataSample s = get_sample();

				log(S_IMU, T_ACCELLERATION, 0.0, 0.0, s.z_acc);
				log(S_IMU, T_FILTER_STATE, s.z_alt, s.z_speed, s.z_acc);

				// log real accelleration but under another type to not interfere with launch simulation
				log(S_IMU, T_SYSLOG, sample.accelerometer[0], sample.accelerometer[1], sample.accelerometer[2]);
#endif
			}
			xSemaphoreGive(spi_semaphore);
		}
		TASK_WAIT_HZ(self, IMU_TASK_HZ);
	}
}

TASK barometer_task(TaskDescriptor_t* self)
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
		log(S_BARO, T_FILTER_STATE, altitude.getState()[0], altitude.getState()[1], altitude.getState()[2]);
		log(S_BARO, T_PRESSURE, sample1.pressure, sample2.pressure);
#endif

		TASK_WAIT_HZ(self, BARO_TASK_HZ);
	}
}

/*
%%{init: {
  "flowchart": {
    "defaultRenderer": "elk",
    "curve": "stepAfter"
  }
} }%%

flowchart TD
    A(((IDLE))) -->|Ignition| B((BOOST))
    B -->|Burnout| C((COAST))
    C -->|Apogee| D((DROGUE)) & a[/Activate main recovery\nActivate backup recovery/]
    D -->|Low Altitude| E((MAIN)) & b[/Release main parachute/]
    E -->|Touchdown| F((LANDED))
    F .->|Reset| A

    A ~~~ B ~~~ C ~~~ D ~~~ E ~~~ F
*/
TASK parachute_task(TaskDescriptor_t* self)
{
	self->last_wake = xTaskGetTickCount();
	RocketState state = RS_IDLE;

	/* TODO:
	 * - Change unit names m/s to MPS
	 * - Change timers from all being referenced to ignition to being referenced to the last state change
	 * - Remove touchdown timer
	 * - Increase accelleration threshold to 3.0g min
	 * - Decrease altitude threshold for main deployment to 400m
	 * - Make this a configuration file or header
	 */

#define PARACHUTE_TASK_HZ 10

#define Z_ACC_BOOST_THRESHOLD_G 3
#define Z_SPEED_BOOST_THRESHOLD_MPS 25.0

#define Z_SPEED_APOGEE_THRESHOLD_MPS -0.01

#define MIN_TIME_TO_1500M_MS 8540

#define Z_ALT_MAIN_DEPLOYMENT_M 400.0

#define Z_ALT_TOUCHDOWN_M 10.0
#define Z_SPEED_STATIONARY_MPS 0.1

#define BOOST_DETECTION_SAMPLE_COUNT 10
#define BURNOUT_DETECTION_SAMPLE_COUNT 10
#define APOGEE_DETECTION_SAMPLE_COUNT 10
#define MAIN_DETECTION_SAMPLE_COUNT 10
#define TOUCHDOWN_DETECTION_SAMPLE_COUNT 10

#define PIN_EJECTION_A PINT6_LS
#define PIN_MAIN_CUTTER PINT2_LS

#define CUTTERS_ON_TIME_MS 2000

	// Interface PINs setup
	pinMode(PIN_EJECTION_A, OUTPUT);
	digitalWrite(PIN_EJECTION_A, 0);
	pinMode(PIN_MAIN_CUTTER, OUTPUT);
	digitalWrite(PIN_MAIN_CUTTER, 0);

	// Sensor data each loop
	float z_speed = 0;
	float z_alt = 0;
	float z_acc = 0;

	// Number of consecutive samples that met the state change condition
	int sample_count = 0;

	bool ejection_active = false;
	int64_t ejection_fire_time = 0;

	bool cutter_active = false;
	int64_t cutter_fire_time = 0;

	while (true) {
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000 / PARACHUTE_TASK_HZ));

		// Get the sensor data, if there is no new sensor data the old sample is
		// used
		LogMessage msg;
		while (xQueueReceive(parachute_msg_queue, &msg, 0) == pdTRUE) {
			if (msg.type == T_FILTER_STATE && msg.payload_type == P_FVEC3) {
				z_alt = msg.payload.fv3.x;
				z_speed = msg.payload.fv3.y;
				z_acc = msg.payload.fv3.z;
			}
		}

		// Non-blocking pyro pin timeout handling — runs every pass regardless
		// of state so it can't stall queue draining
		if (ejection_active && (millis() - ejection_fire_time) >= CUTTERS_ON_TIME_MS) {
			pinMode(PIN_EJECTION_A, OUTPUT);
			digitalWrite(PIN_EJECTION_A, 0);
			ejection_active = false;
		}
		if (cutter_active && (millis() - cutter_fire_time) >= CUTTERS_ON_TIME_MS) {
			pinMode(PIN_MAIN_CUTTER, OUTPUT);
			digitalWrite(PIN_MAIN_CUTTER, 0);
			cutter_active = false;
		}

		switch (state) {
		case RS_IDLE:
			// Detect motor ignition
			if (z_acc >= Z_ACC_BOOST_THRESHOLD_G && z_speed >= Z_SPEED_BOOST_THRESHOLD_MPS) {
				sample_count++;
			} else {
				sample_count = 0;
			}

			if (sample_count >= BOOST_DETECTION_SAMPLE_COUNT) {
				state = RS_BOOST;
				sample_count = 0;
			}

			break;

		case RS_BOOST:
			// Detect motor burnout
			if (z_acc < 0) {
				sample_count++;
			} else {
				sample_count = 0;
			}

			if (sample_count >= BURNOUT_DETECTION_SAMPLE_COUNT) {
				state = RS_COAST;
				sample_count = 0;
			}

			break;

		case RS_COAST:
			// TODO: control aibrakes

			// Detect apogee
			if (z_speed <= Z_SPEED_APOGEE_THRESHOLD_MPS) {
				sample_count++;
			} else {
				sample_count = 0;
			}

			if (sample_count >= APOGEE_DETECTION_SAMPLE_COUNT) {
				// Activate recovery A; pins are cleared later,
				// non-blockingly, by the timeout check above
				analogWrite(PIN_EJECTION_A, 256 / 2);
				ejection_active = true;
				ejection_fire_time = millis();

				state = RS_DROGUE;
				sample_count = 0;
			}

			break;

		case RS_DROGUE:
			// TODO: retract airbrakes

			// Detect main parachute deployment
			if (z_alt <= Z_ALT_MAIN_DEPLOYMENT_M) {
				sample_count++;
			} else {
				sample_count = 0;
			}

			if (sample_count >= MAIN_DETECTION_SAMPLE_COUNT) {
				// Cut main parachute; pin cleared later, non-blockingly
				analogWrite(PIN_MAIN_CUTTER, 256 / 2);
				cutter_active = true;
				cutter_fire_time = millis();

				state = RS_MAIN;
				sample_count = 0;
			}

			break;

		case RS_MAIN:
			// Detect touchdown
			if (z_alt <= Z_ALT_TOUCHDOWN_M || z_speed <= Z_SPEED_STATIONARY_MPS) {
				sample_count++;
			} else {
				sample_count = 0;
			}

			if (sample_count >= TOUCHDOWN_DETECTION_SAMPLE_COUNT) {
				state = RS_TOUCHDOWN;
				sample_count = 0;
			}

			break;

		case RS_TOUCHDOWN:
			break;

		default:
			log(S_PARA, T_SYSLOG, "Unknown rocket state");
			break;
		}

		// log rocket state for telemetry and debugging
		log(S_PARA, T_ROCKET_STATE, state);
	}
}

TASK gps_task(TaskDescriptor_t* self)
{
	self->last_wake = xTaskGetTickCount();
	static GPSData data;

	while (true) {
		gps_update(&data);

		if (data.num_sat < GPS_MIN_SATELLITES) {
			// TODO: print satellites
			log(S_GPS, T_SYSLOG, "Not enough satellites");
		} else {
			// LOG("[GPS]: (%d) pos: (%f, %f), alt: %fm, speed: %fkmh, time:%llu",
			//	data.num_sat, data.lat, data.lon, data.alt, data.kmh, data.unix_time);
			log(S_GPS, T_GPS, data.lat, data.lon);
		}

		TASK_WAIT_HZ(self, GPS_TASK_HZ);
	}
}

TASK ble_formatter_task(TaskDescriptor_t* self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
		// TEST: Blocking receive, if good => apply to all formatter tasks
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		LogMessage msg;

		while (xQueueReceive(ble_msg_queue, &msg, 0) == pdTRUE) {
			ble_send_log_message(msg);
		}
	}
}

TASK lora_formatter_task(TaskDescriptor_t* self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000 / LORA_FMT_TASK_HZ));

		LogMessage msg;

		xSemaphoreTake(lora_tx_packet_semaphore, portMAX_DELAY);
		while (xQueueReceive(lora_msg_queue, &msg, 0) == pdTRUE) {
			switch (msg.type) {
			case T_FILTER_STATE:
				if (msg.payload_type == P_FVEC3) {
					lora_tx_packet.imu.altitude = float16(msg.payload.fv3.x).getBinary();
					lora_tx_packet.imu.vspeed = float16(msg.payload.fv3.y).getBinary();
					lora_tx_packet.imu.dt = msg.timestamp / 1000 - u48le_to_u64(lora_tx_packet.header.tx_time);
				}
				break;
			case T_ORIENTATION: {
				if (msg.payload_type == P_FVEC3) {
					// FIXME: don't repeat this computation here
					float r = msg.payload.fv3.x * 0.0174533;   // roll in radians
					float p = msg.payload.fv3.y * 0.0174533;   // pitch in radians
					float a = acos(cos(p) * cos(r)) * 57.2958; // total pitch from vertical in degrees

					lora_tx_packet.imu.attitude = float16(a).getBinary();
					lora_tx_packet.imu.dt = msg.timestamp / 1000 - u48le_to_u64(lora_tx_packet.header.tx_time);
				}
				break;
			}
			case T_PRESSURE:
				if (msg.payload_type == P_FVEC2) {
					lora_tx_packet.baro.p1 = float16(msg.payload.fv2.x).getBinary();
					lora_tx_packet.baro.p2 = float16(msg.payload.fv2.y).getBinary();
					lora_tx_packet.baro.dt = msg.timestamp / 1000 - u48le_to_u64(lora_tx_packet.header.tx_time);
				}
				break;
			case T_GPS:
				if (msg.payload_type == P_FVEC2) {
					lora_tx_packet.gps.latitude = msg.payload.fv2.x;
					lora_tx_packet.gps.longitude = msg.payload.fv2.y;
					lora_tx_packet.gps.dt = msg.timestamp / 1000 - u48le_to_u64(lora_tx_packet.header.tx_time);
				}
				break;
			case T_ROCKET_STATE:
				if (msg.payload_type == P_ROCKET_STATE) {
					lora_tx_packet.state = (uint8_t)msg.payload.state;
				}
				break;
			default:
				break;
			}
		}
		xSemaphoreGive(lora_tx_packet_semaphore);
	}
}

TASK lora_transmitter_task(TaskDescriptor_t* self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
		// Run the lora radio state machine
		LoRaProtoState state = lora_fc_state_machine();
		String str;
		switch (state) {
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
		Serial.printf("LORA: %s\n", str.c_str());
	}
}

// UART consumer
TASK uart_task(TaskDescriptor_t* self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000 / UART_TASK_HZ));
		LogMessage msg;
		const char* str;
		size_t len = 0;
		while (xQueueReceive(uart_msg_queue, &msg, 0) == pdTRUE) {
			len = logger_message_to_str(&str, &msg);
			Serial.write(str, len);
		}
	}
}

// SD consumer
TASK sd_formatter_task(TaskDescriptor_t* self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000 / SD_FMT_TASK_HZ));
		LogMessage msg;

		while (xQueueReceive(sd_msg_queue, &msg, 0) == pdTRUE) {
			const char* str = NULL;
			size_t len = logger_message_to_str(&str, &msg);
			sdcard_write(str, len);
		}
	}
}

TASK sd_writer_task(TaskDescriptor_t* self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
		sdcard_flush();
		TASK_WAIT_HZ(self, SD_WRITER_TASK_HZ);
	}
}

TASK cmd_handler_task(TaskDescriptor_t* self)
{
	self->last_wake = xTaskGetTickCount();

	while (true) {
		GsCommand cmd;
		if (xQueueReceive(gs_command_queue, &cmd, portMAX_DELAY) != pdTRUE) {
			continue;
		}

		// The log source tells which radio the command came from (S_BLE or S_LORA).
		// Every command is only logged for now: none of them drives an output yet, and in particular
		// the pyro commands must not fire anything until the team decides how they are armed.
		switch (cmd.command) {
		case CMD_NONE:
			log(cmd.src, T_SYSLOG, "Received command: NONE");
			break;
		case CMD_EJECT_A:
			log(cmd.src, T_SYSLOG, "Received command: EJECT_A");
			break;
		case CMD_EJECT_C:
			log(cmd.src, T_SYSLOG, "Received command: EJECT_C");
			break;
		case CMD_CUT_MAIN:
			log(cmd.src, T_SYSLOG, "Received command: CUT_MAIN");
			break;
		case CMD_CAMERAS_ON:
			log(cmd.src, T_SYSLOG, "Received command: CAMERAS_ON");
			break;
		case CMD_CAMERAS_OFF:
			log(cmd.src, T_SYSLOG, "Received command: CAMERAS_OFF");
			break;
		case CMD_SENSOR_CALIBRATION:
			log(cmd.src, T_SYSLOG, "Received command: SENSOR_CALIBRATION");
			break;
		default:
			log(cmd.src, T_SYSLOG, "Received unknown command");
			break;
		}
	}
}
