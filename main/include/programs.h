#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <Arduino.h>

struct WeldProgram
{
    const char *name;
    unsigned long durationMs;
    float targetCurrentMA;
};

extern const WeldProgram WELD_PROGRAMS[];
extern const uint8_t WELD_PROGRAM_COUNT;

extern int selectedProgram;
extern unsigned long svejseStartTime;
extern unsigned long svejseDuration;
extern unsigned long svejseElapsedTime;
extern bool svejseAktiv;

const char *programName(int p);
bool confirmProgram();
void cycleProgram();
float getTargetEnergy();
float getSvejseProgress();
unsigned long getPredictedRemainingTime();
void startSvejse();
bool svejseHandler();
void stopSvejse();

unsigned long getSvejseTime();
unsigned long getSvejseElapsedTime();
float getSvejseTargetCurrentMA();

#endif
