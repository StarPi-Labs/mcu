#include <Arduino.h>

#include "imu.h"
#include "board.h"
#include "logger.h"


extern SPIClass SPI2;
LSM6DSO32Sensor IMU(&SPI2, IMU_CS);
static volatile bool interrupt_flag = false;


/*
// Interrupt handler for IMU FIFO interrupt
static void IRAM_ATTR imu_fifo_interrupt()
{
	interrupt_flag = true;
}
 */


// FIXME: check errors
// returns true on success, false on failure
bool imu_setup()
{
	if (IMU.begin() != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Failed to initialize IMU");
		return false;
	}

	// Should be 0x6C
	uint8_t id;
	if (IMU.ReadID(&id) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Get ID");
		return false;
	}
	if (id != 0x6C) {
		log(S_IMU, T_SYSLOG, "[ERR] ID mismatch");
	}

	if (IMU.Enable_X() !=0 || IMU.Enable_G() != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Enable gyro or accel");
		return false;
	}

	if (IMU.Set_X_FS(IMU_ACCEL_FS) != 0 || IMU.Set_G_FS(IMU_GYRO_FS) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Change FS");
		return false;
	}

#if IMU_FIFO_ENABLE
	// FIFO Configuration
	// TODO: to enable timestamps we need to fork the library and expose an
	//       API to change the correct bits in FIFO_CTRL4
	if (IMU.Set_FIFO_Mode(LSM6DSO32_STREAM_MODE) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Enable FIFO");
		return false;
	}
	if (IMU.Set_FIFO_X_BDR(IMU_FIFO_X_BDR_HZ) != 0 || IMU.Set_FIFO_G_BDR(IMU_FIFO_G_BDR_HZ) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Change buffer data rate");
		return false;
	}
	if (IMU.Set_X_ODR(IMU_FIFO_X_BDR_HZ) != 0 || IMU.Set_G_ODR(IMU_FIFO_G_BDR_HZ) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Change output data rate");
		return false;
	}
	
	// FIFO Interrupt
//	IMU.Set_FIFO_Watermark_Level(IMU_FIFO_WATERMARK);
//	pinMode(IMU_INT1, INPUT_PULLDOWN);
//	attachInterrupt(digitalPinToInterrupt(IMU_INT1), imu_fifo_interrupt, RISING);
#endif

	return true;
}


float imu_calibrate(unsigned int cal_time_ms)
{
	float g_cal = 0.0f;
	int samples = 0;
	int64_t start_time = millis();
	while (millis() - start_time < cal_time_ms) {
		FIFO_Sample sample;
		if (imu_get_sample(&sample) == 0) {
			// Assume the board is stationary
			float ax = (float)sample.accelerometer[0]/1000.0f;
			float ay = (float)sample.accelerometer[1]/1000.0f;
			float az = (float)sample.accelerometer[2]/1000.0f;
			float g = sqrt(ax*ax + ay*ay + az*az);
			g_cal += g;
			samples++;
			delay(5);
		}
	}
	g_cal /= samples;
	return g_cal;
}

// TODO: make this consistent with imu_init
int imu_get_sample(FIFO_Sample *sample)
{
	if (sample == NULL) return -1;

#if IMU_FIFO_ENABLE
	uint16_t n_samples;
	if (IMU.Get_FIFO_Num_Samples(&n_samples) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Get FIFO status");
		return -1;
	}
	if (n_samples == 0) return -1;

	uint16_t x_count = 0, g_count = 0;
	int32_t ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
	FIFO_Sample tmp_sample = {0};

	for (int i = 0; i < n_samples; i++) {
		uint8_t tag = 0;
		if (IMU.Get_FIFO_Tag(&tag) != 0) {
			log(S_IMU, T_SYSLOG, "[ERR] Get FIFO tag");
			return -1;
		}

		switch (tag) {
		case LSM6DSO32_XL_NC_TAG:
			if (IMU.Get_FIFO_X_Axes(tmp_sample.accelerometer) != 0) {
				log(S_IMU, T_SYSLOG, "[ERR] Get X axes");
				return -1;
			}
			ax += tmp_sample.accelerometer[0];
			ay += tmp_sample.accelerometer[1];
			az += tmp_sample.accelerometer[2];
			x_count++;
			break;
		case LSM6DSO32_GYRO_NC_TAG:
			if (IMU.Get_FIFO_G_Axes(tmp_sample.gyroscope) != 0) {
				log(S_IMU, T_SYSLOG, "[ERR] Get G axes");
				return -1;
			}
			gx += tmp_sample.gyroscope[0];
			gy += tmp_sample.gyroscope[1];
			gz += tmp_sample.gyroscope[2];
			g_count++;
			break;
		case LSM6DSO32_TIMESTAMP_TAG:
			// TODO: convert timestamp to unix time (local time in microseconds)
			if (IMU.Get_FIFO_Data(tmp_sample.timestamp) != 0) {
				log(S_IMU, T_SYSLOG, "[ERR] Get timestamp");
				return -1;
			}
			break;
		default:
			break;
		}
	}

	if (x_count == 0 || g_count == 0) {
		// Serial.println("No accelerometer or gyroscope samples in FIFO");
		return -1;
	}

	sample->accelerometer[0] = ax / x_count;
	sample->accelerometer[1] = ay / x_count;
	sample->accelerometer[2] = az / x_count;
	sample->gyroscope[0]     = gx / g_count;
	sample->gyroscope[1]     = gy / g_count;
	sample->gyroscope[2]     = gz / g_count;
	// Serial.printf("FIFO Sample: %d accel samples, %d gyro samples\n", x_count, g_count);

	#else
	if (IMU.Get_X_Axes(sample->accelerometer) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Get X axes");
		return -1;
	}
	if (IMU.Get_G_Axes(sample->gyroscope) != 0) {
		log(S_IMU, T_SYSLOG, "[ERR] Get G axes");
		return -1;

	}
#endif

	return 0;
}
