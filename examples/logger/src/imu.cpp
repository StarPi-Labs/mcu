//https://github.com/stm32duino/LSM6DSO32/tree/main/examples/LSM6DSO32_HelloWorld

#include <Arduino.h>

#include "imu.h"


static SPIClass dev_spi(HSPI);
LSM6DSO32Sensor IMU(&dev_spi, SPI_CS);
static volatile bool interrupt_flag = false;
static FIFO_BATCH fifo;

void print_data(int32_t accelerometer[3], int32_t gyroscope[3], uint8_t timestamp[6]);
void restart_fifo(void);


// Interrupt handler for IMU FIFO interrupt
static void IRAM_ATTR imu_fifo_interrupt()
{
	interrupt_flag = true;
}


void imu_setup()
{
	// Pull the lora module CS high
	pinMode(14, OUTPUT);
	pinMode(SPI_CS, OUTPUT);
	digitalWrite(14, HIGH);
	digitalWrite(SPI_CS, HIGH);

	dev_spi.begin(SPI_SCK, SPI_MISO, SPI_MOSI, -1);

	if (IMU.begin() != 0) {
		while(1) {
			Serial.println("IMU begin failed");
			delay(1000);
		}
	}
	uint8_t id;
	IMU.ReadID(&id);
	delay(2000);
	Serial.printf("IMU ID: 0x%x\n", id);

	IMU.Enable_X();
	IMU.Enable_G();

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
	IMU.Set_FIFO_Mode(LSM6DSO32_STREAM_MODE);
*/

	//interrupt
	//IMU.Set_FIFO_INT1_FIFO_Full(true);
	IMU.Set_FIFO_Watermark_Level(10); // num da capire meglio

	IMU.Write_Reg(LSM6DSO32_INT1_CTRL, 0x08); // Watermark interrput
	IMU.Write_Reg(LSM6DSO32_CTRL10_C, 0x20); // Timestamp fifo

/*
	pinMode(IMU_INT1, INPUT_PULLDOWN);
	attachInterrupt(digitalPinToInterrupt(IMU_INT1), imu_fifo_interrupt, RISING);
*/
}


int imu_get_sample(FIFO_Sample *sample)
{
/*
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
*/
	IMU.Get_X_Axes(sample->accelerometer);
	IMU.Get_G_Axes(sample->gyroscope);
	return 0;
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
