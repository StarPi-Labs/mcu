//https://github.com/stm32duino/LSM6DSO32/tree/main/examples/LSM6DSO32_HelloWorld

#include <Arduino.h>
#include <RadioLib.h>

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
volatile bool in_free_fall = false;
calibration_data_t calib = {0};


void update_sensitivity(const FIFO_Sample &sample){
	if (in_free_fall){
		float sens;
		if (IMU.Get_X_Sensitivity(&sens) == LSM6DSO32_OK) 
			IMU.Set_X_FS(LSM6DSO32_4g);
		return;
	}
	
	int32_t accTot = sample.accelerometer[0]*sample.accelerometer[0] + sample.accelerometer[1]*sample.accelerometer[1] + sample.accelerometer[2]*sample.accelerometer[2];

	
	if (accTot < 4 * 1000) {
		IMU.Set_X_FS(LSM6DSO32_4g);
		return;
	}

	if (accTot < 8 * 1000) {
		IMU.Set_X_FS(LSM6DSO32_8g);		
		return;	
	}

	if (accTot < 16 * 1000) { 
		IMU.Set_X_FS(LSM6DSO32_16g);		
		return;
	}
}


float g_cal = 0.0f;

void IRAM_ATTR check_free_fall()
{
	in_free_fall = true;
}


//FUNZIONE RUBATa DA CATS VEGA - controllare se funziona bene
bool compute_gyro_calibration(const FIFO_Sample *gyro_data, calibration_data_t *calibration) {
	float GYRO_ALLOWED_ERROR_SI = 3.0f; // degrees per second
	uint8_t GYRO_NUM_SAME_VALUE = 200; // number of samples to average for calibration

	static int16_t calibration_counter = 0;
  	static FIFO_Sample first_gyro_data = {.gyroscope = {0, 0, 0}};
  	static FIFO_Sample averaged_gyro_data = {.gyroscope = {0, 0, 0}};

	/* compute gyro error */
	const FIFO_Sample vector_error{.gyroscope = {
		fabsf(first_gyro_data.gyroscope[0] - gyro_data->gyroscope[0]),
		fabsf(first_gyro_data.gyroscope[1] - gyro_data->gyroscope[1]),
		fabsf(first_gyro_data.gyroscope[2] - gyro_data->gyroscope[2])}};

	/* check if the gyro error is inside the bounds
	* if yes, increase counter and compute averaged gyro data
	* if not, reset counter and reset averaged gyro data
	*/
	if ((vector_error.gyroscope[0] < GYRO_ALLOWED_ERROR_SI) && (vector_error.gyroscope[1] < GYRO_ALLOWED_ERROR_SI) &&
		(vector_error.gyroscope[2] < GYRO_ALLOWED_ERROR_SI)) {
		calibration_counter++;
		averaged_gyro_data.gyroscope[0] += gyro_data->gyroscope[0] / static_cast<float>(GYRO_NUM_SAME_VALUE);
		averaged_gyro_data.gyroscope[1] += gyro_data->gyroscope[1] / static_cast<float>(GYRO_NUM_SAME_VALUE);
		averaged_gyro_data.gyroscope[2] += gyro_data->gyroscope[2] / static_cast<float>(GYRO_NUM_SAME_VALUE);
	} else {
		calibration_counter = 0;
		averaged_gyro_data.gyroscope[0] = 0;
		averaged_gyro_data.gyroscope[1] = 0;
		averaged_gyro_data.gyroscope[2] = 0;
		first_gyro_data = *gyro_data;
	}

	/* if the counter achieved the defined value, calibrate gyro */
	if (calibration_counter > GYRO_NUM_SAME_VALUE) {
		memcpy(&calibration->gyro_calib, &averaged_gyro_data, sizeof(averaged_gyro_data));
		return true;
	}

	return false;
}

void correct_gyro(const calibration_data_t *calibration, FIFO_Sample *gyro_data) {
  gyro_data->gyroscope[0] = gyro_data->gyroscope[0] - calibration->gyro_calib.gyroscope[0];
  gyro_data->gyroscope[1] = gyro_data->gyroscope[1] - calibration->gyro_calib.gyroscope[1];
  gyro_data->gyroscope[2] = gyro_data->gyroscope[2] - calibration->gyro_calib.gyroscope[2];
}


int imu_get_sample(FIFO_Sample *sample)
{
	if (sample == NULL) return -1;

#if IMU_FIFO_ENABLE
	uint16_t n_samples;
	IMU.Get_FIFO_Num_Samples(&n_samples);
	if (n_samples == 0) return -1;

	uint8_t x_count = 0, g_count = 0;
	bool xx_ov = false, xy_ov = false, xz_ov = false;
	bool gx_ov = false, gy_ov = false, gz_ov = false;
	FIFO_Sample avg_sample = {0}, tmp_sample = {0};

	for (int i = 0; i < n_samples; i++) {
		uint8_t tag = 0;
		IMU.Get_FIFO_Tag(&tag);

		switch (tag) {
		case LSM6DSO32_XL_NC_TAG:
			IMU.Get_FIFO_X_Axes(tmp_sample.accelerometer);
			xx_ov |= __builtin_add_overflow(avg_sample.accelerometer[0], tmp_sample.accelerometer[0], &avg_sample.accelerometer[0]);
			xy_ov |= __builtin_add_overflow(avg_sample.accelerometer[1], tmp_sample.accelerometer[1], &avg_sample.accelerometer[1]);
			xz_ov |= __builtin_add_overflow(avg_sample.accelerometer[2], tmp_sample.accelerometer[2], &avg_sample.accelerometer[2]);
			x_count++;
			break;
		case LSM6DSO32_GYRO_NC_TAG:
			IMU.Get_FIFO_G_Axes(tmp_sample.gyroscope);
			gx_ov |= __builtin_add_overflow(avg_sample.gyroscope[0], tmp_sample.gyroscope[0], &avg_sample.gyroscope[0]);
			gy_ov |= __builtin_add_overflow(avg_sample.gyroscope[1], tmp_sample.gyroscope[1], &avg_sample.gyroscope[1]);
			gz_ov |= __builtin_add_overflow(avg_sample.gyroscope[2], tmp_sample.gyroscope[2], &avg_sample.gyroscope[2]);
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
	// on overflow return -1, indicating that the sample is invalid
	if (xx_ov || xy_ov || xz_ov || gx_ov || gy_ov || gz_ov) {
		// Serial.println("Overflow detected in FIFO sample accumulation");
		return -1;
	}

	if (x_count == 0 || g_count == 0) {
		// Serial.println("No accelerometer or gyroscope samples in FIFO");
		return -1;
	}

	avg_sample.accelerometer[0] /= x_count;
	avg_sample.accelerometer[1] /= x_count;
	avg_sample.accelerometer[2] /= x_count;
	avg_sample.gyroscope[0] /= g_count;
	avg_sample.gyroscope[1] /= g_count;
	avg_sample.gyroscope[2] /= g_count;
	// Serial.printf("FIFO Sample: %d accel samples, %d gyro samples\n", x_count, g_count);

	*sample = avg_sample;
	return 0;
#else
	IMU.Get_X_Axes(sample->accelerometer);
	IMU.Get_G_Axes(sample->gyroscope);
	return 0;
#endif
}


void calibrate_gyro(){
	bool is_calibrated = false;

	while (is_calibrated){
		FIFO_Sample sample;
		imu_get_sample(&sample);

		is_calibrated = compute_gyro_calibration(&sample, &calib);

	}
}





void imu_setup()
{
	if (IMU.begin() != 0) {
		while(1) {
			ERR(DEST_UART, "Failed to initialize IMU");
			delay(1000);
		}
	}

	// Should be 0x6C
	uint8_t id;
	IMU.ReadID(&id);
	if (id != 0x6C) {
		while(1) {
			ERR(DEST_UART, "IMU ID mismatch: expected 0x6C, got", id);
			delay(1000);
		}
	}

	IMU.Enable_X();
	IMU.Enable_G();

	IMU.Set_X_FS(LSM6DSO32_32g);
	IMU.Set_G_FS(LSM6DSO32_2000dps);

	IMU.Enable_Free_Fall_Detection(LSM6DSO32_INT1_PIN);

	//da controllare che siano giusti i parametri
	IMU.Set_Free_Fall_Threshold(500);// 500 - 438 - 312 mg
	IMU.Set_Free_Fall_Duration(63); //cicli ODR   


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
	attachInterrupt(digitalPinToInterrupt(IMU_INT1), check_free_fall, RISING);
	
	

#endif

	const int cal_time_ms = 5000;
	int samples = 0;
	for (int t = 0; t < cal_time_ms;) {
		// get current time in freertos tick
		TickType_t start_tick = xTaskGetTickCount();

		FIFO_Sample sample;
		if (imu_get_sample(&sample) == 0) {
			// TODO: should be a.z*cos(alpha), this assumes board is perfectly horizontal
			g_cal += (float)sample.accelerometer[2]/1000.0f;
			samples++;
		}

		t += (xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS;
	}
	g_cal /= samples;

	calibrate_gyro();
}


