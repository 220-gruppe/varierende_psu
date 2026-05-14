#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <Arduino.h>

struct Program{
    const char* name;
    float       resistance_ohm;
    float       mass_kg;
    float       targetTemp_C;
};

extern const         Program PROGRAMS[];
extern const int     NUM_PROGRAMS;

extern int           selectedProgram;
extern unsigned long svejseStartTime;
extern unsigned long svejseDuration;
extern bool          svejseAktiv;

const char *programName(int p);
bool confirmProgram();
void cycleProgram();
float getTargetEnergy();
float getSvejseProgress();
unsigned long getPredictedRemainingTime();
void startSvejse();
bool svejseHandler();
void stopSvejse();


#endif
