//https://github.com/stm32duino/LSM6DSO32/tree/main/examples/LSM6DSO32_HelloWorld

#include <Arduino.h>
#include <LSM6DSO32Sensor.h>
#include <SPI.h>

#define SPI_MOSI 11
#define SPI_MISO 13
#define SPI_SCK 12
#define SPI_CS 15
#define IMU_INT1 10

//da board.h del branch logging 
#define IMU_TASK_HZ    100
#define IMU_FIFO_X_BDR_HZ  (IMU_TASK_HZ * 1.20f)
#define IMU_FIFO_G_BDR_HZ  (IMU_TASK_HZ * 1.20f)
//


SPIClass dev_spi(HSPI);
LSM6DSO32Sensor IMU(&dev_spi, SPI_CS);

void print_data(int32_t accelerometer[3], int32_t gyroscope[3], uint8_t timestamp[6]);
void IRAM_ATTR fifo_interrupt();

volatile bool interrupt_flag = false;

typedef struct {
  int32_t accelerometer[3]; 
  int32_t gyroscope[3];
  uint8_t timestamp[6];
} FIFO_Sample;

typedef struct {
  FIFO_Sample fifo_batch[148]; //3072 bytes / () = 114
  int index = 0; 
} FIFO_BATCH;


FIFO_BATCH fifo;

int imu_get_sample(FIFO_Sample *sample);

void restart_fifo();

float g_cal = 0.0f;


void setup()
{
	if (IMU.begin() != 0) {
		while(1) {
			Serial.println("Failed to initialize IMU");
			delay(1000);
		}
	}

	// Should be 0x6C
	uint8_t id;
	IMU.ReadID(&id);
	if (id != 0x6C) {
		while(1) {
			Serial.println("IMU ID mismatch: expected 0x6C, got " + String(id));
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
}


int imu_get_sample(FIFO_Sample *sample);


void loop()
{
  FIFO_Sample sample;
  if (imu_get_sample(&sample) == 0) {
    print_data(sample.accelerometer, sample.gyroscope, sample.timestamp);
  } else {
    Serial.println("Failed to get sample");
  }
}

void IRAM_ATTR fifo_interrupt(){
  interrupt_flag = true;
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


void restart_fifo(){
    IMU.Set_FIFO_Mode(LSM6DSO32_BYPASS_MODE); 

    IMU.Disable_X();  
    IMU.Disable_G();

    IMU.Enable_X();  
    IMU.Enable_G();

    IMU.Set_FIFO_Mode(LSM6DSO32_BYPASS_MODE); 
}



void print_data(int32_t accelerometer[3], int32_t gyroscope[3], uint8_t timestamp[6]){
  Serial.print("| Acc[mg]: ");
  Serial.print(accelerometer[0]);
  Serial.print(" ");
  Serial.print(accelerometer[1]);
  Serial.print(" ");
  Serial.print(accelerometer[2]);
  Serial.print(" | Gyr[mdps]: ");
  Serial.print(gyroscope[0]);
  Serial.print(" ");
  Serial.print(gyroscope[1]);
  Serial.print(" ");
  Serial.print(gyroscope[2]);

  uint32_t ts = (timestamp[3] << 24) | (timestamp[2] << 16) | (timestamp[1] << 8) | timestamp[0];

  Serial.print("Timestamp: ");
  Serial.println((float)ts / 1000 * 25); //converti in ms 

  Serial.println(" |");
}