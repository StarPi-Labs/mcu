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
void imu_setup()
{
	if (IMU.begin() != 0) {
		while(1) {
			log(S_IMU, T_SYSLOG, "[ERR] Failed to initialize IMU");
			delay(1000);
		}
	}

	// Should be 0x6C
	uint8_t id;
	IMU.ReadID(&id);
	if (id != 0x6C) {
		while(1) {
			//ERR("IMU ID mismatch: expected 0x6C, got %lu", (uint32_t)id);
			log(S_IMU, T_SYSLOG, "[ERR] ID mismatch");
			delay(1000);
		}
	}

	IMU.Enable_X();
	IMU.Enable_G();

	/*
	// TODO: define in imu.h
	IMU.Set_X_FS(LSM6DSO32_32g);
	IMU.Set_G_FS(LSM6DSO32_2000dps);

	// TODO: define in imu.h
	*/

#if IMU_FIFO_ENABLE
	// FIFO Configuration
	// TODO: to enable timestamps we need to fork the library and expose an
	//       API to change the correct bits in FIFO_CTRL4
	IMU.Set_FIFO_Mode(LSM6DSO32_STREAM_MODE);
	IMU.Set_FIFO_X_BDR(IMU_FIFO_X_BDR_HZ);
	IMU.Set_FIFO_G_BDR(IMU_FIFO_G_BDR_HZ);
	IMU.Set_X_ODR(IMU_FIFO_X_BDR_HZ);
	IMU.Set_G_ODR(IMU_FIFO_G_BDR_HZ);

	// FIFO Interrupt
//	IMU.Set_FIFO_Watermark_Level(IMU_FIFO_WATERMARK);
//	pinMode(IMU_INT1, INPUT_PULLDOWN);
//	attachInterrupt(digitalPinToInterrupt(IMU_INT1), imu_fifo_interrupt, RISING);
#endif
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


int imu_get_sample(FIFO_Sample *sample)
{
	if (sample == NULL) return -1;

#if IMU_FIFO_ENABLE
	uint16_t n_samples;
	IMU.Get_FIFO_Num_Samples(&n_samples);
	if (n_samples == 0) return -1;

	uint16_t x_count = 0, g_count = 0;
	int32_t ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
	FIFO_Sample tmp_sample = {0};

	for (int i = 0; i < n_samples; i++) {
		uint8_t tag = 0;
		IMU.Get_FIFO_Tag(&tag);

		switch (tag) {
		case LSM6DSO32_XL_NC_TAG:
			IMU.Get_FIFO_X_Axes(tmp_sample.accelerometer);
			ax += tmp_sample.accelerometer[0];
			ay += tmp_sample.accelerometer[1];
			az += tmp_sample.accelerometer[2];
			x_count++;
			break;
		case LSM6DSO32_GYRO_NC_TAG:
			IMU.Get_FIFO_G_Axes(tmp_sample.gyroscope);
			gx += tmp_sample.gyroscope[0];
			gy += tmp_sample.gyroscope[1];
			gz += tmp_sample.gyroscope[2];
			g_count++;
			break;
		case LSM6DSO32_TIMESTAMP_TAG:
			// TODO: convert timestamp to unix time (local time in microseconds)
			IMU.Get_FIFO_Data(tmp_sample.timestamp);
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

	return 0;
#else
	IMU.Get_X_Axes(sample->accelerometer);
	IMU.Get_G_Axes(sample->gyroscope);
	return 0;
#endif
}
