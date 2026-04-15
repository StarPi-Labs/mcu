#pragma once


// SPI2 Bus, FSPI
#define SPI2_MOSI 11
#define SPI2_MISO 13
#define SPI2_SCK  12

// IMU Pins
#define IMU_CS    15
#define IMU_INT1  10

// LoRa Pins
#define LORA_BUSY 5
#define LORA_RST  6
#define LORA_DIO1 7
#define LORA_CS   14


// IMU watermak level, this defines how many samples we read from the FIFO before
// each interrupt
// IMU_FIFO_x_BDR_HZ / (IMU_FIFO_WATERMARK/2) should be >= IMU_TASK_HZ
#define IMU_FIFO_ENABLE    1
#define IMU_FIFO_WATERMARK 4
#define IMU_FIFO_X_BDR_HZ  1660.0f
#define IMU_FIFO_G_BDR_HZ  1660.0f


// Task Timings
#define IMU_TASK_HZ    100
#define LOGGER_TASK_HZ 500