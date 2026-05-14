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
void startSvejse();
void stopSvejse();
bool svejseHandler();
unsigned long getSvejseTime();
unsigned long getSvejseElapsedTime();
float getSvejseTargetCurrentMA();

#endif
