#ifndef NUMPAD_H
#define NUMPAD_H

#include "Arduino.h"
#include <Adafruit_MPR121.h>
#include <Wire.h>

void setupNumpad();
void numpadLogik();
String getTyped();
bool isUserDone();
void resetUserDone();
void clearTyped();
void resetNumpad();

#endif
