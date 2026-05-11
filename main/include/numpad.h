#ifndef NUMPAD_H
#define NUMPAD_H

#include "Arduino.h"
#include <Adafruit_MPR121.h>
#include <Wire.h>

using NumpadOverride = bool (*)(int activeBit);
void setNumpadOverride(NumpadOverride callback);

void setupNumpad();
void numpadLogik();
String getTyped();
bool isUserDone();
void resetNumpad();

#endif
