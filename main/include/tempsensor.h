#ifndef TEMPS_H
#define TEMPS_H

#include <DFRobot_MLX90614.h>
#include <Wire.h>

extern DFRobot_MLX90614_I2C sensor;  // instantiate an object to drive our sensor

void setupTempSensor();
float takeTempMeasurement();    

#endif  
