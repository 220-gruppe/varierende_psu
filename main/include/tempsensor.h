#ifndef TEMPS_H
#define TEMPS_H

#include <Arduino.h>
#include <Wire.h>

extern float AVG_TEMP;

void setupTempSensor();
float takeTempMeasurement();    

#endif  
