#include "tempsensor.h"

DFRobot_MLX90614_I2C sensor(0x5A, &Wire); 

void setupTempSensor() {
  while (NO_ERR != sensor.begin()) {
    Serial.println("Fejl ved initialisering af temp sensor, prøver igen...");
    delay(300);
  }
  Serial.println("TEMPS sensor OK");
}

float takeTempMeasurement() {

  const int samples = 20;
  float sum = 0;
  sensor.enterSleepMode(false);
  delay(200);
  for (int i = 0; i < samples; i++) {
    sum += sensor.getObjectTempCelsius();
    delay(100);
  }
  sensor.enterSleepMode(true);

  return sum / samples;
}