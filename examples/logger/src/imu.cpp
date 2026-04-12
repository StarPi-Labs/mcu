//https://github.com/stm32duino/LSM6DSO32/tree/main/examples/LSM6DSO32_HelloWorld

#include <Arduino.h>
#include <RadioLib.h>

#include "imu.h"


static SPIClass dev_spi(FSPI);
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


void disable_lora_sx1262()
{
    // Replace these with your actual GPIO numbers
    int lora_cs = 14;
    int lora_dio1 = 7;  // Example GPIO
    int lora_busy = 5;  // Example GPIO
    int lora_reset = 6; // Example GPIO

    // Module(CS, DI01, RST, BUSY, SPI)
    Module lora_module(lora_cs, lora_dio1, lora_reset, lora_busy, dev_spi);
    SX1262 radio(&lora_module);

    Serial.println("Initializing SX1262 to force sleep...");
    int state = radio.begin();
    
    if (state == RADIOLIB_ERR_NONE) {
        // false = cold sleep (disables SPI high-impedance mode/buffer)
        radio.sleep(false); 
        Serial.println("SX1262 successfully sent to sleep.");
    } else {
        Serial.printf("Error: Could not talk to LoRa (Code %d). MISO may still be blocked.\n", state);
    }

    pinMode(lora_cs, OUTPUT);
    digitalWrite(lora_cs, HIGH);
}


// Do some fucking black magic to de-stuck the IMU
// turns out it may not be needed but better to have it than not
void resuscitate_imu()
{
    Serial.println("[RESCUE] Starting Phase 1: Hardware Bit-Bang Flush...");

    // 1. CRITICAL: Detach pins from the ESP32 Hardware SPI controller!
    dev_spi.end();

    // 2. CRITICAL: Lock out the LoRa module so it ignores our bit-banging
    int lora_cs = 14; 
    pinMode(lora_cs, OUTPUT);
    digitalWrite(lora_cs, HIGH);

    // Now we can safely take manual GPIO control of the SPI pins
    pinMode(SPI_CS, OUTPUT);
    pinMode(SPI_SCK, OUTPUT);
    pinMode(SPI_MOSI, OUTPUT);
    
    // Default idle state for SPI Mode 0 (Clock LOW)
    digitalWrite(SPI_CS, HIGH);
    digitalWrite(SPI_SCK, LOW); 
    digitalWrite(SPI_MOSI, LOW);
    delay(10);

    // Toggle CS to shake the internal multiplexer
    for (int i = 0; i < 5; i++) {
        digitalWrite(SPI_CS, LOW); delay(5);
        digitalWrite(SPI_CS, HIGH); delay(5);
    }

    // Flush the shift register
    digitalWrite(SPI_CS, LOW);
    delay(1);
    for (int i = 0; i < 16; i++) {
        digitalWrite(SPI_SCK, HIGH); delayMicroseconds(50);
        digitalWrite(SPI_SCK, LOW); delayMicroseconds(50);
    }
    digitalWrite(SPI_CS, HIGH);
    delay(10);

    Serial.println("[RESCUE] Phase 2: Software Reset & I2C Disable...");
    
    // 3. Hand control back to the ESP32's hardware SPI controller
    dev_spi.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    
    // IMPORTANT: Switching to SPI_MODE0 (CPOL=0, CPHA=0)
    dev_spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    
    // 4. Send SW_RESET to CTRL3_C (0x12)
    digitalWrite(SPI_CS, LOW);
    dev_spi.transfer(0x12); // Write to CTRL3_C
    dev_spi.transfer(0x81); // BOOT + SW_RESET
    digitalWrite(SPI_CS, HIGH);
    
    Serial.println("re-booting the IMU");
    delay(1000); // Wait for the IMU to reboot

    // THE PULL-UP TEST AND RAW READ
    pinMode(SPI_MISO, INPUT_PULLUP);

    // 5. HARD LOCK TO SPI MODE (Disable I2C)
    // Register CTRL4_C (0x13), Bit 2 (0x04) is I2C_disable
    digitalWrite(SPI_CS, LOW);
    dev_spi.transfer(0x13); // Write to CTRL4_C
    dev_spi.transfer(0x04); // Set I2C_disable bit
    digitalWrite(SPI_CS, HIGH);

    delay(10);

    // 6. Raw read test
    digitalWrite(SPI_CS, LOW);
    dev_spi.transfer(0x0F | 0x80); // Read WHO_AM_I (0x0F + Read Bit 0x80)
    uint8_t raw_id = dev_spi.transfer(0x00);
    digitalWrite(SPI_CS, HIGH);
    
    dev_spi.endTransaction();

    Serial.printf("[RESCUE] Post-Reset Raw IMU ID: 0x%02X\n", raw_id);
}


void imu_setup()
{
	dev_spi.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
	dev_spi.setFrequency(1000000);

	disable_lora_sx1262();

//	resuscitate_imu();

	if (IMU.begin() != 0) {
		while(1) {
			Serial.println("IMU begin failed");
			delay(1000);
		}
	}
	
	// Should be 0x6C
	uint8_t id;
	IMU.ReadID(&id);
	Serial.printf("IMU ID: 0x%x\n", id);
	
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
