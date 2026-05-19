#include <Arduino.h>
#include <FreeRTOS.h>
#include <MadgwickAHRS.h>

#include "KalmanFilter.hpp"
#include "Logging.h"
#include "barometer.h"
#include "board.h"
#include "imu.h"
#include "lora.h"
#include "sdcard.h"
#include "task.h"

SPIClass SPI2(FSPI);
TwoWire I2C1(0);

// State filter for orientation estimation, used in the IMU task
Madgwick orientation;

// Altitude and veritcal velocity state filter, used to sense when to deploy
// the parachute
KalmanFilter altitude;

extern float g_cal;

DECLARE_STATIC_SEMAPHORE(spi_semaphore);
// TODO: add semaphore for i2c once we connect the pitot

DECLARE_STATIC_TASK(imu_task);
DECLARE_STATIC_TASK(barometer_task);
DECLARE_STATIC_TASK(lora_task);

void setup(void)
{

  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.println("Initialized");

  I2C1.setPins(I2C1_SDA, I2C1_SCL);
  I2C1.begin();
  I2C1.setClock(100000);
  SPI2.begin(SPI2_SCK, SPI2_MISO, SPI2_MOSI, -1);
  // TODO: Set speed

  mcu::log::addTarget(
      "Serial",
      [](std::string_view msg) {
        assert(xPortInIsrContext() == pdFALSE &&
               "Logging to Serial is not allowed from an ISR context");

        Serial.write(reinterpret_cast<const uint8_t*>(msg.data()), msg.size());
      },
      tskIDLE_PRIORITY + 1, 2048);

  mcu::log::addTarget(
      "SD Card",
      [](std::string_view msg) {
        if (sdcard_open_log()) {
          sdcard_write_data(msg.data(), msg.size());
          sdcard_close_log();
        }
      },
      tskIDLE_PRIORITY + 1, 4096);

  mcu::log::init();

  imu_setup();
  altitude.setG(g_cal);

  barometer_setup();
  lora_setup();

  if (!sdcard_init()) {
    while (true) {
      Serial.println("SD init failed");
      delay(500);
    }
  }

  // creates folder /session_<session_num>
  if (!sdcard_start_session()) {
    while (true) {
      Serial.println("Error creating the SD session folder");
      delay(500);
    }
  }

  INIT_STATIC_SEMAPHORE(spi_semaphore);
  if (spi_semaphore == NULL) {
    while (true) {
      Serial.println("Error creating semaphore");
      delay(500);
    }
  }

  INIT_STATIC_TASK(imu_task, "imu", NULL, tskIDLE_PRIORITY + 10, 0);
  INIT_STATIC_TASK(barometer_task, "barometer", NULL, tskIDLE_PRIORITY + 9, 0);
  INIT_STATIC_TASK(lora_task, "lora", NULL, tskIDLE_PRIORITY + 10, 1);

  if (!TASK_IS_INITIALIZED(imu_task) || !TASK_IS_INITIALIZED(barometer_task) ||
      !TASK_IS_INITIALIZED(lora_task)) {
    while (true) {
      Serial.println("Error creating tasks");
      delay(500);
    }
  }
}

void loop(void)
{
  Serial.println("LOOP");
  delay(1000);
}

TASK imu_task(TaskDescriptor_t* self)
{
  self->last_wake = xTaskGetTickCount();
  FIFO_Sample sample;
  orientation.begin(IMU_TASK_HZ);

  while (true) {
    if (xSemaphoreTake(spi_semaphore, portMAX_DELAY) == pdTRUE &&
        imu_get_sample(&sample) == 0) {
      // Update the relative orientation of the board using
      // the Madgwick filter, readings are in mg and mdps, so
      // conversion is needed
      orientation.updateIMU((float)sample.gyroscope[0] / 1000.0f,
                            (float)sample.gyroscope[1] / 1000.0f,
                            (float)sample.gyroscope[2] / 1000.0f,
                            (float)sample.accelerometer[0] / 1000.0f,
                            (float)sample.accelerometer[1] / 1000.0f,
                            (float)sample.accelerometer[2] / 1000.0f);

      // Update the altitude and vertical velocity estimation
      // with the inertial data
      altitude.predict((float)sample.accelerometer[2] / 1000.0f,
                       orientation.getPitch(),
                       false // TODO: airbrake trigger
      );

      mcu_log_info("[IMU]: Orientation ({}, {}, {})\n", orientation.getRoll(),
                   orientation.getPitch(), orientation.getYaw());

      xSemaphoreGive(spi_semaphore);
    }
    TASK_WAIT_HZ(self, IMU_TASK_HZ);
  }
}

TASK barometer_task(TaskDescriptor_t* self)
{
  self->last_wake = xTaskGetTickCount();
  BaroData sample1, sample2;

  while (true) {
    barometer_read(&sample1, &sample2);

    // FIXME: is the median really the best way to fuse the two barometer
    // readings?
    float alt = (sample1.altitude + sample2.altitude) / 2.0f;

    // Update the altitude and vertical velocity estimation with the barometer
    // data
    altitude.update(alt);

    mcu_log_info("[BARO]: Altitude {}\n", altitude.getState()[0]);

    TASK_WAIT_HZ(self, BARO_TASK_HZ);
  }
}

TASK lora_task(TaskDescriptor_t* self)
{
  self->last_wake = xTaskGetTickCount();

  while (true) {
    if (xSemaphoreTake(spi_semaphore, portMAX_DELAY) == pdTRUE) {
      if (lora_is_transmission_done()) {
        mcu_log_info("[LORA]: Transmission done\n");
        // TODO: transmit the actual message
        lora_start_transmission(LoRaPayload{0}.bytes, sizeof(LoRaPayload));
      } else {
        mcu_log_info("[LORA]: Transmission in progress\n");
      }
      xSemaphoreGive(spi_semaphore);
    }
    TASK_WAIT_HZ(self, LORA_TASK_HZ);
  }
}
