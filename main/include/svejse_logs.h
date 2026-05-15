#ifndef SVEJSE_LOGS_H
#define SVEJSE_LOGS_H

#include <Arduino.h>

enum class SvejsningStatus
{
    Approved = 0,
    NotApproved = 1
};

extern const float OUTPUT_RESISTANCE_OHM;
extern const unsigned long SVEJSNING_MEASUREMENT_INTERVAL_MS;

float calculatedOutputEnergy();
bool wasApproved();
void saveSvejsningResult();
bool initializeSvejsningLog(const String &uid, int programNumber, unsigned long startMs, float targetCurrentMA, unsigned long targetDurationMs);
bool appendSvejsningMeasurement(unsigned long timestampMs, float currentMA, float voltageV, float totalJoule);
void finalizeSvejsningLog(const String &status, float totalJoule, float targetCurrentMA, const String &avgTemp, float svejsningTime, unsigned long targetDurationMs);
void printSdCardContents();
void removeSvejsningTestLogs();
bool writeSvejseLog(const String &svejsningResult,
                    const String &calculatedEnergy,
                    const String &avgTemp,
                    float svejsningTime);
void LogSvejseData();

#endif 
