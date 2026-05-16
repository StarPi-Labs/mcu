#include <Arduino.h>
#include <Logging.h>

void setup()
{
  Serial.begin(115200);
  mcu::log::init();
  mcu_log_info("System initialized\n");
}

void loop()
{
  static uint32_t counter = 0;
  mcu_log_info("Loop iteration, counter: {}\n", counter++);
  delay(1000);
}