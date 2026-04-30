#ifndef TEMPS_H
#define TEMPS_H

#include <DFRobot_MLX90614.h>

extern DFRobot_MLX90614_I2C sensor;  // instantiate an object to drive our sensor

float takeTempMeasurement() {

  const int samples = 20;
  float sum = 0;
  // sensor.enterSleepMode(false);
  delay(300);
  for (int i = 0; i < samples; i++) {
    sum += sensor.getObjectTempCelsius();
    delay(200);
  }
  // sensor.enterSleepMode(true);

  return sum / samples;
}

#endif  