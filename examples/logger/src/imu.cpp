//https://github.com/stm32duino/LSM6DSO32/tree/main/examples/LSM6DSO32_HelloWorld

#include <Arduino.h>
#include <RadioLib.h>

#include "imu.h"
#include "board.h"
#include "logger.h"


extern SPIClass SPI2;
LSM6DSO32Sensor IMU(&SPI2, IMU_CS);
static volatile bool interrupt_flag = false;
static FIFO_BATCH fifo;

void print_data(int32_t accelerometer[3], int32_t gyroscope[3], uint8_t timestamp[6]);
void restart_fifo(void);


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
	IMU.Write_Reg(LSM6DSO32_CTRL10_C, 0x20); // Timestamp fifo

/*
	// TODO: define in imu.h
	IMU.Set_X_FS(LSM6DSO32_32g);
	IMU.Set_G_FS(LSM6DSO32_2000dps);

	// TODO: define in imu.h
	IMU.Set_X_ODR(6667.0f); //max
	IMU.Set_G_ODR(6667.0f);

	// fifo
	// TODO: define in imu.h
	IMU.Set_FIFO_X_BDR(6667.0f); //max
	IMU.Set_FIFO_G_BDR(6667.0f);
	*/

	//IMU.Set_FIFO_INT1_FIFO_Full(true);
	IMU.Set_FIFO_Mode(LSM6DSO32_STREAM_MODE);
	IMU.Set_FIFO_Watermark_Level(10); // num da capire meglio
	//IMU.Write_Reg(LSM6DSO32_INT1_CTRL, 0x08); // Watermark interrput

	//interrupt
	pinMode(IMU_INT1, INPUT_PULLDOWN);
	attachInterrupt(digitalPinToInterrupt(IMU_INT1), imu_fifo_interrupt, RISING);
}


int imu_get_sample(FIFO_Sample *sample)
{
	if (interrupt_flag == false || sample == NULL) return -1;

	interrupt_flag = false;
	fifo.index = 0; //reset batch index

	uint16_t nSamples;
	IMU.Get_FIFO_Num_Samples(&nSamples);

	bool X_received = false, G_received = false, timestamp_received = false;

	for (int i = 0; i < nSamples; i++) {
		uint8_t tag = 0;
		IMU.Get_FIFO_Tag(&tag);

		switch (tag) {
		case LSM6DSO32_XL_NC_TAG:
			IMU.Get_FIFO_X_Axes(fifo.fifo_batch[fifo.index].accelerometer);
			X_received = true;
			break;
		case LSM6DSO32_GYRO_NC_TAG:
			IMU.Get_FIFO_G_Axes(fifo.fifo_batch[fifo.index].gyroscope);
			G_received = true;
			break;
		case LSM6DSO32_TIMESTAMP_TAG:
			// TODO: convert timestamp to unix time (local time in microseconds)
			IMU.Get_FIFO_Data(fifo.fifo_batch[fifo.index].timestamp);
			timestamp_received = true;
			break;
		}

		if (G_received && timestamp_received) { //se è l'ultimo af essere ricevuto incrementa indice
			fifo.index++;
			X_received = false;
			G_received = false;
			timestamp_received = false;
		}

		if (fifo.index >= 148) {
			break;
		}
	}

	// For now return the first sample
	// TODO: either interpolate samples or return the whole batch
	// TODO: return the info about what data was received
	*sample = fifo.fifo_batch[0];
	return 0;
/*
	IMU.Get_X_Axes(sample->accelerometer);
	IMU.Get_G_Axes(sample->gyroscope);
	return 0;
*/
}


// Restart the IMU FIFO (bypass mode)
void imu_restart()
{
	IMU.Set_FIFO_Mode(LSM6DSO32_BYPASS_MODE);

	IMU.Disable_X();
	IMU.Disable_G();

	IMU.Enable_X();
	IMU.Enable_G();

	IMU.Set_FIFO_Mode(LSM6DSO32_BYPASS_MODE);
}
