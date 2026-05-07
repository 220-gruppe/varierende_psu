#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <Arduino.h>

extern int selectedProgram;
extern unsigned long svejseStartTime;
extern unsigned long svejseDuration;
extern bool svejseAktiv;

const char *programName(int p);
bool confirmProgram();
void cycleProgram();
void startSvejse();
void stopSvejse();
bool svejseHandler();
float getTargetEnergy();
unsigned long getSvejseTime();

#endif
