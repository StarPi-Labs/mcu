//https://github.com/stm32duino/LSM6DSO32/tree/main/examples/LSM6DSO32_HelloWorld

#include <Arduino.h>
#include <RadioLib.h>

#include "imu.h"
#include "board.h"
#include "logger.h"


extern SPIClass SPI2;
LSM6DSO32Sensor IMU(&SPI2, IMU_CS);
static volatile bool interrupt_flag = false;


// Interrupt handler for IMU FIFO interrupt
static void IRAM_ATTR imu_fifo_interrupt()
{
	interrupt_flag = true;
}


/*
// Do some fucking black magic to de-stuck the IMU
// turns out it may not be needed but better to have it than not
// THIS CODE IS AI GENERATED
void resuscitate_imu()
{
	Serial.println("[RESCUE] Starting Phase 1: Hardware Bit-Bang Flush...");

	// 1. CRITICAL: Detach pins from the ESP32 Hardware SPI controller!
	SPI2.end();

	// Now we can safely take manual GPIO control of the SPI pins
	pinMode(IMU_CS, OUTPUT);
	pinMode(SPI2_SCK, OUTPUT);
	pinMode(SPI2_MOSI, OUTPUT);

	// Default idle state for SPI Mode 0 (Clock LOW)
	digitalWrite(IMU_CS, HIGH);
	digitalWrite(SPI2_SCK, LOW);
	digitalWrite(SPI2_MOSI, LOW);
	delay(10);

	// Toggle CS to shake the internal multiplexer
	for (int i = 0; i < 5; i++) {
		digitalWrite(IMU_CS, LOW); delay(5);
		digitalWrite(IMU_CS, HIGH); delay(5);
	}

	// Flush the shift register
	digitalWrite(IMU_CS, LOW);
	delay(1);
	for (int i = 0; i < 16; i++) {
		digitalWrite(SPI2_SCK, HIGH); delayMicroseconds(50);
		digitalWrite(SPI2_SCK, LOW); delayMicroseconds(50);
	}
	digitalWrite(IMU_CS, HIGH);
	delay(10);

	Serial.println("[RESCUE] Phase 2: Software Reset & I2C Disable...");

	// 3. Hand control back to the ESP32's hardware SPI controller
	SPI2.begin(SPI2_SCK, SPI2_MISO, SPI2_MOSI);

	// IMPORTANT: Switching to SPI_MODE0 (CPOL=0, CPHA=0)
	SPI2.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

	// 4. Send SW_RESET to CTRL3_C (0x12)
	digitalWrite(IMU_CS, LOW);
	SPI2.transfer(0x12); // Write to CTRL3_C
	SPI2.transfer(0x81); // BOOT + SW_RESET
	digitalWrite(IMU_CS, HIGH);

	Serial.println("re-booting the IMU");
	delay(1000); // Wait for the IMU to reboot

	// THE PULL-UP TEST AND RAW READ
	pinMode(SPI2_MISO, INPUT_PULLUP);

	// 5. HARD LOCK TO SPI MODE (Disable I2C)
	// Register CTRL4_C (0x13), Bit 2 (0x04) is I2C_disable
	digitalWrite(IMU_CS, LOW);
	SPI2.transfer(0x13); // Write to CTRL4_C
	SPI2.transfer(0x04); // Set I2C_disable bit
	digitalWrite(IMU_CS, HIGH);

	delay(10);

	// 6. Raw read test
	digitalWrite(IMU_CS, LOW);
	SPI2.transfer(0x0F | 0x80); // Read WHO_AM_I (0x0F + Read Bit 0x80)
	uint8_t raw_id = SPI2.transfer(0x00);
	digitalWrite(IMU_CS, HIGH);

	SPI2.endTransaction();

	Serial.printf("[RESCUE] Post-Reset Raw IMU ID: 0x%02X\n", raw_id);
}
*/


void imu_setup()
{
//	resuscitate_imu();

	if (IMU.begin() != 0) {
		while(1) {
			ERR("Failed to initialize IMU");
			delay(1000);
		}
	}

	// Should be 0x6C
	uint8_t id;
	IMU.ReadID(&id);
	if (id != 0x6C) {
		while(1) {
			ERR("IMU ID mismatch: expected 0x6C, got", id);
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
	IMU.Set_X_ODR(6667.0f); //max
	IMU.Set_G_ODR(6667.0f);
	*/

#if IMU_FIFO_ENABLE
	// FIFO Configuration
	// TODO: to enable timestamps we need to fork the library and expose an
	//       API to change the correct bits in FIFO_CTRL4
	IMU.Set_FIFO_Mode(LSM6DSO32_STREAM_MODE);
	IMU.Set_FIFO_X_BDR(IMU_FIFO_X_BDR_HZ);
	IMU.Set_FIFO_G_BDR(IMU_FIFO_G_BDR_HZ);
	// IMU.Set_FIFO_INT1_FIFO_Full(true);
	IMU.Set_FIFO_Watermark_Level(IMU_FIFO_WATERMARK);

	// FIFO Interrupt
	pinMode(IMU_INT1, INPUT_PULLDOWN);
	attachInterrupt(digitalPinToInterrupt(IMU_INT1), imu_fifo_interrupt, RISING);
#endif
}


int imu_get_sample(FIFO_Sample *sample)
{
	if (sample == NULL) return -1;

#if IMU_FIFO_ENABLE
	if (interrupt_flag == false) return -1;

	interrupt_flag = false;

	uint16_t n_samples;
	IMU.Get_FIFO_Num_Samples(&n_samples);

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

	if (x_count == 0 && g_count == 0) {
		// Serial.println("No accelerometer or gyroscope samples in FIFO");
		return -1;
	}

	avg_sample.accelerometer[0] /= x_count;
	avg_sample.accelerometer[1] /= x_count;
	avg_sample.accelerometer[2] /= x_count;
	avg_sample.gyroscope[0] /= g_count;
	avg_sample.gyroscope[1] /= g_count;
	avg_sample.gyroscope[2] /= g_count;
	Serial.printf("FIFO Sample: %d accel samples, %d gyro samples\n", x_count, g_count);

	// TODO: return the info about what data was received
	*sample = avg_sample;
	return 0;
#else
	IMU.Get_X_Axes(sample->accelerometer);
	IMU.Get_G_Axes(sample->gyroscope);
	return 0;
#endif
}
