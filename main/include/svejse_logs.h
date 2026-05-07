#ifndef SVEJSE_LOGS_H
#define SVEJSE_LOGS_H

#include <Arduino.h>

extern float heatInput;

enum class SvejsningStatus
{
    Approved = 0,
    NotApproved = 1
};

float calculatedOutputEnergy();
float getTargetEnergy();
SvejsningStatus GetSvejsningStatus();
bool wasApproved();
void saveSvejsningResult();
bool writeSvejseLog(const String &svejsningResult,
                    const String &calculatedEnergy,
                    const String &avgTemp,
                    float svejsningTime);
void LogSvejseData();

#endif 