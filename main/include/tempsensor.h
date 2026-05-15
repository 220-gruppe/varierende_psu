#ifndef TEMPS_H
#define TEMPS_H

#include <Arduino.h>
#include <Wire.h>

extern float AVG_TEMP;
extern const int TEMP_SENSOR_SAMPLES;
extern const unsigned long TEMP_SENSOR_SAMPLE_DELAY_MS;
extern const float TEMP_SENSOR_MAX_VALID_C;

void setupTempSensor();
float takeTempMeasurement();    

#endif  
