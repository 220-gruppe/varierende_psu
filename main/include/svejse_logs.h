#ifndef SVEJSE_LOGS_H
#define SVEJSE_LOGS_H

#include <Arduino.h>

enum class SvejsningStatus
{
    Approved = 0,
    NotApproved = 1
};

float calculatedOutputEnergy();
float getTargetEnergy();
bool wasApproved();
void saveSvejsningResult();
bool initializeSvejsningLog(const String &uid, int programNumber, unsigned long startMs, float targetJoule);
bool appendSvejsningMeasurement(unsigned long timestampMs, float currentMA, float voltageV, float totalJoule);
void finalizeSvejsningLog(const String &status, float totalJoule, float targetJoule, const String &avgTemp, float svejsningTime);
bool writeSvejseLog(const String &svejsningResult,
                    const String &calculatedEnergy,
                    const String &avgTemp,
                    float svejsningTime);
void LogSvejseData();

#endif 