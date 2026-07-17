#pragma once


// SPI2 Bus, FSPI
#define SPI2_MOSI 11
#define SPI2_MISO 13
#define SPI2_SCK  12

// I2C1 Bus
#define I2C1_SDA 42
#define I2C1_SCL 45

// IMU Pins
#define IMU_CS    15
#define IMU_INT1  10

// LoRa Pins
#define LORA_BUSY 5
#define LORA_RST  6
#define LORA_DIO1 7
#define LORA_CS   14

// UART2, GPS Pins
#define GPS_TX    17
#define GPS_RX    18

// Peripheral Pins
#define PINT1_HS 3
#define PINT1_LS 4
#define PINT2_HS 39
#define PINT2_LS 38
#define PINT3_HS 41
#define PINT3_LS 40
#define PINT4_LS 35
#define PINT5_LS 36
#define PINT6_LS 37


// Task Timings
#define IMU_TASK_HZ       100
#define BARO_TASK_HZ      20
#define GPS_TASK_HZ       2
#define UART_TASK_HZ      50
#define LORA_FMT_TASK_HZ  20
#define LORA_TX_TASK_HZ   50
#define SD_FMT_TASK_HZ    50
#define SD_WRITER_TASK_HZ 10


// IMU FIFO Configuration
// IMU_FIFO_x_BDR_HZ should be >= IMU_TASK_HZ to always have samples to read but
// not too high to cause overflow in the FIFO and/or excessive CPU usage in the
// IMU task
#define IMU_FIFO_ENABLE    1
#define IMU_FIFO_WATERMARK 4
#define IMU_FIFO_X_BDR_HZ  (IMU_TASK_HZ * 1.20f)
#define IMU_FIFO_G_BDR_HZ  (IMU_TASK_HZ * 1.20f)


// Barometer configuration
// Oversampling rate: this affects the preision and sampling time, for the two
// barometers ULTRA_HIGH means about 70ms per both samples, high 50ms
// OSR_STANDARD works for 20Hz sampling
//#define BARO_OSR OSR_STANDARD
#define BARO_OSR OSR_ULTRA_HIGH


// LoRa configuration
#define LORA_OUTPUT_POWER     0     // dBm, da capire
#define LORA_SPREADING_FACTOR 7     // minimo spread factor
#define LORA_CODING_RATE      5     // 4 + 1, 1 bit su 8 di correzione di errore
#define LORA_CRC_BYTES        1


// GPS configuration
#define GPS_MIN_SATELLITES         3
#define GPS_SERIAL_BUFFER_SIZE     1024
#define GPS_SERIAL_READ_TIMEOUT_MS 100
